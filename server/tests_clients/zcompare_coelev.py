#!/usr/bin/env python3
# Co-elevation comparison (no movement): connect many players, use the GUI to
# find two that SPAWNED on the same tile where a linemate is already present,
# then have one issue Incantation and check whether the OTHER (non-initiator)
# receives any unsolicited response line.
import socket, select, subprocess, time, signal

MINE="/home/fred/Projects/Uni/Year2/Uni_mirrors/Zappy/server/build/bin/zappy_server"
REF ="/home/fred/Projects/Uni/Year2/Uni_mirrors/Zappy/server/test_bins/zappy_server"
W=H=10
NPLAYERS=70

class Sock:
    def __init__(self,port):
        self.s=socket.create_connection(("127.0.0.1",port)); self.buf=b""
    def send(self,l): self.s.sendall((l+"\n").encode())
    def line(self,t=3.0):
        while b"\n" not in self.buf:
            r,_,_=select.select([self.s],[],[],t)
            if not r: return None
            d=self.s.recv(4096)
            if not d: return "<closed>"
            self.buf+=d
        l,_,self.buf=self.buf.partition(b"\n"); return l.decode(errors="replace")
    def resp(self,t=4.0):
        while True:
            x=self.line(t)
            if x is None: return None
            if x.startswith("message ") or x.startswith("eject:"): continue
            return x
    def cmd(self,l,t=4.0): self.send(l); return self.resp(t)
    def silent(self,secs=1.5):
        r,_,_=select.select([self.s],[],[],secs)
        return None if not r else self.s.recv(4096).decode(errors="replace")
    def close(self):
        try:self.s.close()
        except:pass

class Gui(Sock):
    def __init__(self,port):
        super().__init__(port); self.pos={}; self.order=[]
        self.send("GRAPHIC"); self.pump(1.0)
    def pump(self,secs=0.5):
        end=time.time()+secs
        while time.time()<end:
            r,_,_=select.select([self.s],[],[],0.1)
            if not r: continue
            self.buf+=self.s.recv(4096)
            while b"\n" in self.buf:
                l,_,self.buf=self.buf.partition(b"\n"); self._p(l.decode(errors="replace"))
    def _p(self,l):
        p=l.split()
        if len(p)>=5 and p[0]=="pnw":
            fd=int(p[1][1:]); self.pos[fd]=(int(p[2]),int(p[3]))
            if fd not in self.order: self.order.append(fd)
        elif len(p)>=5 and p[0]=="ppo":
            self.pos[int(p[1][1:])]=(int(p[2]),int(p[3]))

def launch(b,port):
    p=subprocess.Popen([b,"-p",str(port),"-x",str(W),"-y",str(H),
        "-n","t1","t2","-c",str(NPLAYERS),"-f","100"],
        stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    time.sleep(1.0); return p
def kill(p):
    try:p.send_signal(signal.SIGINT);time.sleep(0.2);p.kill()
    except:pass

def suite(binary, base):
    R={}; p=launch(binary,base)
    try:
        gui=Gui(base)
        # burst-connect all players fast (avoid starvation during setup),
        # then map by pnw arrival order == connection order
        conns=[]
        for i in range(NPLAYERS):
            team="t1" if i%2==0 else "t2"
            c=Sock(base); c.line(); c.send(team); c.line(); c.line()
            conns.append(c)
        gui.pump(2.5)
        socks={}  # fd -> Sock
        for i,fd in enumerate(gui.order):
            if i < len(conns): socks[fd]=conns[i]
        # group fds by tile
        bytile={}
        for fd,(x,y) in gui.pos.items():
            bytile.setdefault((x,y),[]).append(fd)
        pairs=[fds for fds in bytile.values() if len(fds)>=2]
        R["players_connected"]=len(socks)
        R["co_located_tiles"]=len(pairs)
        # find a co-located pair on a tile that already holds a linemate
        chosen=None
        for fds in pairs:
            init=socks[fds[0]]
            lk=init.cmd("Look")
            if lk and "linemate" in lk.strip("[]").split(",")[0]:
                chosen=(fds[0],fds[1]); break
        if chosen is None:
            R["result"]="NO co-located linemate tile (rerun)"; return R
        ini,obs=socks[chosen[0]],socks[chosen[1]]
        ini.silent(0.3); obs.silent(0.3)
        ini.send("Incantation")
        R["init_underway"]=ini.resp(4.0)
        R["OBSERVER_during"]=obs.silent(2.0)   # KEY: non-initiator gets a line?
        R["init_result"]=ini.resp(6.0)
        R["OBSERVER_after"]=obs.silent(2.0)    # KEY
        bl=obs.cmd("Look"); R["observer_look_tiles"]=len(bl.split(",")) if bl else None
        for c in conns: c.close()
        gui.close()
    finally: kill(p)
    return R

print("MINE..."); m=suite(MINE,4360)
print("REF...");  r=suite(REF,4370)
keys=list(dict.fromkeys(list(m)+list(r)))
print(f"\n{'KEY':22} {'MINE':<28} {'REFERENCE':<28} MATCH")
print("-"*92)
for k in keys:
    mv=repr(m.get(k)); rv=repr(r.get(k))
    print(f"{k:22} {mv[:27]:<28} {rv[:27]:<28} {'OK' if m.get(k)==r.get(k) else 'DIFF'}")
print("\nOBSERVER_* = bytes the non-initiator received (None = silent / correct).")
print("observer_look_tiles: 4=level1, 9=level2 (confirms silent level-up).")
