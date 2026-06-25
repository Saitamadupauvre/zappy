import socket, select, subprocess, time, sys, os, signal

MINE = "/home/fred/Projects/Uni/Year2/Uni_mirrors/Zappy/server/build/bin/zappy_server"
REF  = "/home/fred/Projects/Uni/Year2/Uni_mirrors/Zappy/server/test_bins/zappy_server"

class Client:
    def __init__(self, port, team):
        self.s = socket.create_connection(("127.0.0.1", port)); self.buf=b""
        self.readline(); self._send(team); self.readline(); self.readline()
    def _send(self,l): self.s.sendall((l+"\n").encode())
    def readline(self, timeout=3.0):
        while b"\n" not in self.buf:
            r,_,_=select.select([self.s],[],[],timeout)
            if not r: return None
            d=self.s.recv(4096)
            if not d: return "<closed>"
            self.buf+=d
        line,_,self.buf=self.buf.partition(b"\n"); return line.decode(errors="replace")
    def resp(self, timeout=4.0):
        while True:
            x=self.readline(timeout)
            if x is None: return None
            if x.startswith("message ") or x.startswith("eject:"): continue
            return x
    def cmd(self,l,timeout=4.0): self._send(l); return self.resp(timeout)
    def silent(self, secs=1.5):
        r,_,_=select.select([self.s],[],[],secs)
        return None if not r else self.s.recv(4096).decode(errors="replace")
    def close(self):
        try: self.s.close()
        except: pass

def launch(binary, port, w, h):
    p = subprocess.Popen([binary,"-p",str(port),"-x",str(w),"-y",str(h),
        "-n","t1","t2","-c","5","-f","100"],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(1.0); return p

def kill(p):
    try: p.send_signal(signal.SIGINT); time.sleep(0.2); p.kill()
    except: pass

def run_suite(binary, base):
    R={}
    # --- 10x10 single client battery ---
    p=launch(binary, base, 10,10)
    try:
        a=Client(base,"t1")
        inv=a.cmd("Inventory")
        R["inv_fmt"]= inv and inv.startswith("[food ") and inv.endswith("]")
        R["connect_nbr"]=a.cmd("Connect_nbr")
        R["forward"]=a.cmd("Forward"); R["right"]=a.cmd("Right"); R["left"]=a.cmd("Left")
        R["badcmd"]=a.cmd("Boguscommand")
        R["set_noarg"]=a.cmd("Set")
        R["take_bad"]=a.cmd("Take notastone")
        # broadcast self-echo: response must be exactly 'ok', nothing else queued
        a._send("Broadcast hi"); R["bcast_resp"]=a.resp()
        R["bcast_selfecho"]=a.silent(1.0)   # expect None (no message echoed back)
        # single-player incantation: walk to a linemate tile then incant
        got=False
        for _ in range(60):
            lk=a.cmd("Look"); t0=lk.strip("[]").split(",")[0]
            if "linemate" in t0: got=True; break
            a.cmd("Forward")
        if got:
            a._send("Incantation")
            R["incant_underway"]=a.resp(4.0)
            R["incant_result"]=a.resp(6.0)
        else:
            R["incant_underway"]=R["incant_result"]="NO_LINEMATE_TILE"
        a.close()
    finally: kill(p)
    return R

print("running MINE..."); mine=run_suite(MINE, 4300)
print("running REF...");  ref =run_suite(REF, 4310)

keys=list(dict.fromkeys(list(mine)+list(ref)))
print(f"\n{'KEY':24} {'MINE':<30} {'REFERENCE':<30} {'MATCH'}")
print("-"*100)
for k in keys:
    m=repr(mine.get(k)); r=repr(ref.get(k))
    print(f"{k:24} {m[:29]:<30} {r[:29]:<30} {'OK' if mine.get(k)==ref.get(k) else 'DIFF'}")
