#!/usr/bin/env python3
# Compare take(pickup) / Look / Inventory behaviour: mine vs school reference.
# Asserts behaviour (response codes, inventory deltas, tile counts) rather than
# absolute values, so it is robust to each server's random map.
import socket, select, subprocess, time, signal

MINE="/home/fred/Projects/Uni/Year2/Uni_mirrors/Zappy/server/build/bin/zappy_server"
REF ="/home/fred/Projects/Uni/Year2/Uni_mirrors/Zappy/server/test_bins/zappy_server"
STONES=["food","linemate","deraumere","sibur","mendiane","phiras","thystame"]

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
    def close(self):
        try:self.s.close()
        except:pass

def parse_inv(s):
    d={}
    for tok in s.strip("[] ").split(","):
        p=tok.split()
        if len(p)==2:
            try: d[p[0]]=int(p[1])
            except ValueError: pass
    return d

def tile0_tokens(look):
    first=look.strip("[]").split(",")[0]
    return first.split()

def launch(b,port):
    p=subprocess.Popen([b,"-p",str(port),"-x","10","-y","10","-n","t1","t2",
        "-c","5","-f","100"],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    time.sleep(1.0); return p
def kill(p):
    try:p.send_signal(signal.SIGINT);time.sleep(0.2);p.kill()
    except:pass

def suite(binary, port):
    R={}; p=launch(binary,port)
    try:
        a=Sock(port); a.line(); a.send("t1"); a.line(); a.line()

        # --- Inventory ---
        inv=a.cmd("Inventory"); d=parse_inv(inv)
        R["inv_fields"]=len(d)                       # expect 7
        R["inv_has_food"]="food" in d                # expect True
        R["inv_all_stones_present"]=all(s in d for s in STONES)

        # --- Look level 1 ---
        look=a.cmd("Look")
        R["look_lvl1_tiles"]=len(look.strip("[]").split(","))   # expect 4
        R["look_tile0_has_player"]="player" in tile0_tokens(look)

        # --- find a tile holding a resource, then TAKE it ---
        res=None; pre_toks=[]
        for _ in range(40):
            pre_toks=tile0_tokens(a.cmd("Look"))
            stones=[t for t in pre_toks if t in STONES]
            if stones: res=stones[0]; break
            a.cmd("Forward")
        R["found_resource_tile"]=res is not None
        if res:
            before=parse_inv(a.cmd("Inventory")).get(res,0)
            R["take_present"]=a.cmd(f"Take {res}")          # expect ok
            after=parse_inv(a.cmd("Inventory")).get(res,0)
            R["take_increments_inv"]=(after==before+1)
            toks_after=tile0_tokens(a.cmd("Look"))
            R["take_removes_from_tile"]=(toks_after.count(res)==pre_toks.count(res)-1)
            # --- SET it back down ---
            R["set_drop"]=a.cmd(f"Set {res}")               # expect ok
            back=parse_inv(a.cmd("Inventory")).get(res,0)
            R["set_decrements_inv"]=(back==after-1)
            R["set_returns_to_tile"]=(res in tile0_tokens(a.cmd("Look")))

        # --- TAKE a resource that is NOT on the tile ---
        absent=[s for s in STONES if s not in tile0_tokens(a.cmd("Look"))]
        if absent:
            R["take_absent"]=a.cmd(f"Take {absent[0]}")     # expect ko
        # --- bad resource name ---
        R["take_badname"]=a.cmd("Take notastone")           # expect ko
        a.close()
    finally: kill(p)
    return R

print("MINE..."); m=suite(MINE,4380)
print("REF...");  r=suite(REF,4390)
keys=list(dict.fromkeys(list(m)+list(r)))
print(f"\n{'KEY':26} {'MINE':<20} {'REFERENCE':<20} MATCH")
print("-"*80)
for k in keys:
    mv=repr(m.get(k)); rv=repr(r.get(k))
    print(f"{k:26} {mv[:19]:<20} {rv[:19]:<20} {'OK' if m.get(k)==r.get(k) else 'DIFF'}")
