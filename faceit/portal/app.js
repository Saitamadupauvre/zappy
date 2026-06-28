const api = (path, opts = {}) => {
  const token = localStorage.getItem('token');
  return fetch('/api' + path, {
    headers: { 'Authorization': token ? `Bearer ${token}` : '', 'Content-Type': 'application/json', ...opts.headers },
    ...opts,
  });
};

function showSection(id) {
  document.querySelectorAll('main section').forEach(s => s.style.display = 'none');
  document.getElementById(id).style.display = '';
  if (id === 'ai') loadAIs();
  if (id === 'servers') loadServers();
  if (id === 'guis') loadGUIs();
  if (id === 'rooms') { loadRoomServerSelect(); refreshRooms(); }
  if (id === 'live') { loadLiveGuiSelect(); refreshLive(); }
  if (id === 'history') refreshHistory();
  if (id === 'leaderboard') refreshLeaderboard();
}

function logout() {
  localStorage.removeItem('token');
  document.getElementById('user-label').textContent = '';
  document.getElementById('logout-btn').style.display = 'none';
  showSection('auth');
}

async function login() {
  const username = document.getElementById('auth-username').value;
  const password = document.getElementById('auth-password').value;
  const form = new URLSearchParams({ username, password });
  const res = await fetch('/api/login', { method: 'POST', body: form });
  const data = await res.json();
  if (res.ok) {
    localStorage.setItem('token', data.access_token);
    document.getElementById('user-label').textContent = username;
    document.getElementById('logout-btn').style.display = '';
    document.getElementById('auth-msg').textContent = '';
    showSection('rooms');
  } else {
    document.getElementById('auth-msg').textContent = data.detail || 'Login failed';
  }
}

async function register() {
  const username = document.getElementById('auth-username').value;
  const password = document.getElementById('auth-password').value;
  const res = await api('/register', { method: 'POST', body: JSON.stringify({ username, password }) });
  const data = await res.json();
  if (res.ok) {
    localStorage.setItem('token', data.access_token);
    document.getElementById('user-label').textContent = username;
    document.getElementById('logout-btn').style.display = '';
    document.getElementById('auth-msg').textContent = '';
    showSection('rooms');
  } else {
    document.getElementById('auth-msg').textContent = data.detail || 'Registration failed';
  }
}

// ── AIs ───────────────────────────────────────────────────────────────────────

async function loadAIs() {
  const res = await api('/ai/list');
  if (!res.ok) return;
  const ais = await res.json();
  const tbody = document.querySelector('#ai-table tbody');
  tbody.innerHTML = ais.length
    ? ais.map(a =>
        `<tr><td>${a.name}</td><td>${a.mmr.toFixed(1)}</td><td>${a.matches_played}</td>
         <td><button onclick="deleteAI(${a.id})">Delete</button></td></tr>`
      ).join('')
    : '<tr><td colspan="4">No AIs uploaded</td></tr>';
}

async function deleteAI(id) {
  await api(`/ai/${id}`, { method: 'DELETE' });
  loadAIs();
}

// ── Servers ───────────────────────────────────────────────────────────────────

async function loadServers() {
  const res = await api('/server/list');
  if (!res.ok) return;
  const servers = await res.json();
  const tbody = document.querySelector('#server-table tbody');
  tbody.innerHTML = servers.length
    ? servers.map(s =>
        `<tr><td>${s.name}</td><td>${new Date(s.uploaded_at).toLocaleString()}</td>
         <td><button onclick="deleteServer(${s.id})">Delete</button></td></tr>`
      ).join('')
    : '<tr><td colspan="3">No server binaries uploaded</td></tr>';
}

async function deleteServer(id) {
  await api(`/server/${id}`, { method: 'DELETE' });
  loadServers();
}

// ── Rooms ─────────────────────────────────────────────────────────────────────

async function loadRoomServerSelect() {
  const res = await api('/server/list');
  if (!res.ok) return;
  const servers = await res.json();
  const sel = document.getElementById('room-server-select');
  sel.innerHTML = '<option value="">Select server binary</option>' +
    servers.map(s => `<option value="${s.id}">${s.name}</option>`).join('');
}

async function createRoom() {
  const server_binary_id = parseInt(document.getElementById('room-server-select').value);
  const max_players = parseInt(document.getElementById('room-mode-select').value);
  if (!server_binary_id) return;
  const res = await api('/rooms', { method: 'POST', body: JSON.stringify({ server_binary_id, max_players }) });
  const data = await res.json();
  document.getElementById('room-create-msg').textContent = res.ok
    ? `Room #${data.id} created!`
    : (data.detail || 'Failed to create room');
  if (res.ok) refreshRooms();
}

async function refreshRooms() {
  const [roomsRes, aisRes] = await Promise.all([api('/rooms'), api('/ai/list')]);
  const rooms = roomsRes.ok ? await roomsRes.json() : [];
  const ais = aisRes.ok ? await aisRes.json() : [];

  const aiOptions = ais.length
    ? ais.map(a => `<option value="${a.id}">${a.name} (${a.mmr.toFixed(1)})</option>`).join('')
    : '<option value="">No AIs uploaded</option>';

  const container = document.getElementById('rooms-list');
  if (!rooms.length) {
    container.innerHTML = '<p>No open rooms. Host one above.</p>';
    return;
  }

  container.innerHTML = rooms.map(r => {
    const playerRows = r.players.map(p =>
      `<tr><td>${p.ai_name}</td><td>${p.username}</td><td>${p.mmr.toFixed(1)}</td></tr>`
    ).join('');
    const emptySlots = r.max_players - r.players.length;
    const emptyRows = Array(emptySlots).fill('<tr><td colspan="3" class="empty-slot">— waiting —</td></tr>').join('');

    return `
    <div class="room-card">
      <div class="room-header">
        <strong>Room #${r.id}</strong>
        <span class="room-slots">${r.slots}/${r.max_players}</span>
        <span class="room-mode">${r.max_players === 2 ? '1v1' : '4-player FFA'}</span>
        <span class="room-host">Host: ${r.host_username}</span>
        <span class="room-date">${new Date(r.created_at).toLocaleString()}</span>
      </div>
      <table class="room-players">
        <thead><tr><th>AI</th><th>Player</th><th>MMR</th></tr></thead>
        <tbody>${playerRows}${emptyRows}</tbody>
      </table>
      <div class="room-actions">
        <select id="room-ai-${r.id}">${aiOptions}</select>
        <button onclick="joinRoom(${r.id})">Join</button>
        <button onclick="leaveRoom(${r.id})">Leave</button>
        <button onclick="closeRoom(${r.id})">Close</button>
        <span id="room-msg-${r.id}" class="room-msg"></span>
      </div>
    </div>`;
  }).join('');
}

async function joinRoom(roomId) {
  const sel = document.getElementById(`room-ai-${roomId}`);
  const ai_binary_id = parseInt(sel.value);
  if (!ai_binary_id) { setRoomMsg(roomId, 'Select an AI first'); return; }
  const res = await api('/queue/join', { method: 'POST', body: JSON.stringify({ ai_binary_id, room_id: roomId }) });
  const data = await res.json();
  setRoomMsg(roomId, res.ok ? `Joined (${data.players}/4)` : (data.detail || 'Failed'));
  if (res.ok) refreshRooms();
}

async function leaveRoom(roomId) {
  const res = await api('/queue/leave', { method: 'DELETE' });
  setRoomMsg(roomId, res.ok ? 'Left room' : 'Failed');
  if (res.ok) refreshRooms();
}

async function closeRoom(id) {
  const res = await api(`/rooms/${id}`, { method: 'DELETE' });
  if (!res.ok) {
    const data = await res.json().catch(() => ({}));
    alert(data.detail || 'Failed to close room');
  }
  refreshRooms();
}

function setRoomMsg(roomId, msg) {
  const el = document.getElementById(`room-msg-${roomId}`);
  if (el) el.textContent = msg;
}

// ── GUIs ──────────────────────────────────────────────────────────────────────

async function loadGUIs() {
  const res = await api('/gui/list');
  if (!res.ok) return;
  const guis = await res.json();
  const tbody = document.querySelector('#gui-table tbody');
  tbody.innerHTML = guis.length
    ? guis.map(g =>
        `<tr><td>${g.name}</td><td>${new Date(g.uploaded_at).toLocaleString()}</td>
         <td><button onclick="deleteGUI(${g.id})">Delete</button></td></tr>`
      ).join('')
    : '<tr><td colspan="3">No GUI binaries uploaded</td></tr>';
}

async function deleteGUI(id) {
  await api(`/gui/${id}`, { method: 'DELETE' });
  loadGUIs();
}

async function loadLiveGuiSelect() {
  const res = await api('/gui/list');
  if (!res.ok) return;
  const guis = await res.json();
  const sel = document.getElementById('live-gui-select');
  sel.innerHTML = guis.length
    ? '<option value="">Select GUI</option>' + guis.map(g => `<option value="${g.id}">${g.name}</option>`).join('')
    : '<option value="">No GUI uploaded</option>';
}

async function launchGUI(matchId) {
  const gui_id = parseInt(document.getElementById('live-gui-select').value);
  if (!gui_id) { alert('Select a GUI binary first'); return; }
  const res = await api(`/gui/launch/${matchId}?gui_id=${gui_id}`, { method: 'POST' });
  const data = await res.json();
  if (res.ok) alert(`GUI launched on port ${data.port}`);
  else alert(data.detail || 'Launch failed');
}

// ── Match History ─────────────────────────────────────────────────────────────

async function refreshHistory() {
  const res = await api('/matches?limit=50');
  if (!res.ok) return;
  const matches = await res.json();
  const container = document.getElementById('history-list');
  const finished = matches.filter(m => m.status === 'finished');
  if (!finished.length) {
    container.innerHTML = '<p>No finished matches yet.</p>';
    return;
  }
  container.innerHTML = finished.map(m => {
    const date = m.finished_at ? new Date(m.finished_at).toLocaleString() : '—';
    const rows = m.participants.map(p => {
      const delta = p.mmr_after != null ? p.mmr_after - p.mmr_before : null;
      const deltaStr = delta != null
        ? `<span class="${delta >= 0 ? 'mmr-gain' : 'mmr-loss'}">${delta >= 0 ? '+' : ''}${delta.toFixed(1)}</span>`
        : '—';
      const isWinner = p.team_name === m.winner_team;
      return `<tr${isWinner ? ' class="match-winner"' : ''}>
        <td>${isWinner ? '🏆 ' : ''}${p.ai_name}</td>
        <td>${p.mmr_before != null ? p.mmr_before.toFixed(1) : '—'}</td>
        <td>${p.mmr_after  != null ? p.mmr_after.toFixed(1)  : '—'}</td>
        <td>${deltaStr}</td>
      </tr>`;
    }).join('');
    return `
    <div class="match-card">
      <div class="match-header">
        <strong>Match #${m.id}</strong>
        <span class="match-winner-label">Winner: ${m.winner_team || '—'}</span>
        <span class="match-date">${date}</span>
      </div>
      <table class="match-table">
        <thead><tr><th>AI</th><th>MMR Before</th><th>MMR After</th><th>Change</th></tr></thead>
        <tbody>${rows}</tbody>
      </table>
    </div>`;
  }).join('');
}

// ── Live / Leaderboard ────────────────────────────────────────────────────────

async function refreshLive() {
  const res = await fetch('/api/matches/live');
  const matches = await res.json();
  const tbody = document.querySelector('#live-table tbody');
  tbody.innerHTML = matches.length
    ? matches.map(m =>
        `<tr><td>${m.id}</td><td><strong>${m.port}</strong></td>
         <td>${new Date(m.started_at).toLocaleTimeString()}</td>
         <td><button onclick="launchGUI(${m.id})">Launch GUI</button></td></tr>`
      ).join('')
    : '<tr><td colspan="4">No live matches</td></tr>';
}

async function refreshLeaderboard() {
  const res = await fetch('/api/leaderboard');
  const rows = await res.json();
  const tbody = document.querySelector('#lb-table tbody');
  tbody.innerHTML = rows.map((r, i) =>
    `<tr><td>#${i + 1}</td><td>${r.name}</td><td>${r.username}</td><td>${r.mmr}</td><td>${r.matches_played}</td></tr>`
  ).join('');
}

// ── Init ──────────────────────────────────────────────────────────────────────

document.getElementById('gui-upload-form').addEventListener('submit', async e => {
  e.preventDefault();
  const name = document.getElementById('gui-name').value;
  const file = document.getElementById('gui-file').files[0];
  const form = new FormData();
  form.append('name', name);
  form.append('file', file);
  const token = localStorage.getItem('token');
  const res = await fetch('/api/gui/upload', { method: 'POST', headers: { Authorization: `Bearer ${token}` }, body: form });
  if (res.ok) {
    document.getElementById('gui-upload-msg').textContent = 'Uploaded!';
    loadGUIs();
  } else {
    const err = await res.json().catch(() => ({}));
    document.getElementById('gui-upload-msg').textContent = `Error ${res.status}: ${err.detail || 'Upload failed'}`;
  }
});

document.getElementById('upload-form').addEventListener('submit', async e => {
  e.preventDefault();
  const name = document.getElementById('ai-name').value;
  const file = document.getElementById('ai-file').files[0];
  const form = new FormData();
  form.append('name', name);
  form.append('file', file);
  const token = localStorage.getItem('token');
  const res = await fetch('/api/ai/upload', { method: 'POST', headers: { Authorization: `Bearer ${token}` }, body: form });
  document.getElementById('upload-msg').textContent = res.ok ? 'Uploaded!' : 'Upload failed';
  if (res.ok) loadAIs();
});

document.getElementById('server-upload-form').addEventListener('submit', async e => {
  e.preventDefault();
  const name = document.getElementById('server-name').value;
  const file = document.getElementById('server-file').files[0];
  const form = new FormData();
  form.append('name', name);
  form.append('file', file);
  const token = localStorage.getItem('token');
  const res = await fetch('/api/server/upload', { method: 'POST', headers: { Authorization: `Bearer ${token}` }, body: form });
  if (res.ok) {
    document.getElementById('server-upload-msg').textContent = 'Uploaded!';
    loadServers();
  } else {
    const err = await res.json().catch(() => ({}));
    document.getElementById('server-upload-msg').textContent = `Error ${res.status}: ${err.detail || 'Upload failed'}`;
  }
});

if (localStorage.getItem('token')) {
  api('/me').then(r => r.json()).then(u => {
    if (u.username) {
      document.getElementById('user-label').textContent = u.username;
      document.getElementById('logout-btn').style.display = '';
    }
  }).catch(() => {});
}
