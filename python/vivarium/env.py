"""Gymnasium-shaped Python API; fallback is educational, not bit-identical to C++."""
import math, random
_IDS={"BallPush-v0":"BallPush","BallPush":"BallPush","Navigate-v0":"Navigate","Navigate":"Navigate","PoleBalance-v0":"PoleBalance","PoleBalance":"PoleBalance","StackBlocks-v0":"StackBlocks","StackBlocks":"StackBlocks"}
class Env:
    def __init__(self,env_id,seed=None,headless=True,max_steps=None):
        if env_id not in _IDS: raise ValueError(f"Unknown environment {env_id!r}; available: {', '.join(sorted(_IDS))}")
        self.env_id=env_id;self.kind=_IDS[env_id];self.headless=bool(headless);self.max_steps=int(max_steps or (1000 if self.kind=="PoleBalance" else 500))
        if self.max_steps<1: raise ValueError("max_steps must be positive")
        self._seed=0 if seed is None else int(seed);self._rng=random.Random(self._seed);self._closed=False;self._native=None
        try:
            from . import _vivarium
            self._native=_vivarium.make_native(env_id)
        except ImportError: pass
        self.reset(seed=self._seed)
    def reset(self,seed=None,options=None):
        del options
        self._closed=False
        if seed is not None:self._seed=int(seed)
        self._steps=0
        if self._native is not None: obs=list(self._native.reset(self._seed))
        else:
            self._rng.seed(self._seed)
            if self.kind=="BallPush":self._state=[1.2,3.,0.,0.,self._rng.uniform(6,7.5),self._rng.uniform(1.5,4.5),0.,0.,9.,3.]
            elif self.kind=="Navigate":self._state=[self._rng.uniform(.5,2),self._rng.uniform(.5,2),9.,9.]
            elif self.kind=="PoleBalance":self._state=[self._rng.uniform(-.04,.04),0.,0.,0.]
            else:self._height=0;self._top_x=5.;self._placement_x=5.;self._state=[]
            obs=self._observe()
        return obs,{"seed":self._seed}
    def _observe(self):
        if self.kind=="StackBlocks":return [float(self._height),self._placement_x,self._top_x,self._steps/self.max_steps,4.-self._height,self._top_x-self._placement_x]
        if self.kind=="Navigate":
            x,y,tx,ty=self._state;return [x,y,tx,ty,tx-x,ty-y]
        return list(self._state)
    def _actions(self,action,n):
        a=[float(x) for x in action]
        if len(a)!=n:raise ValueError(f"Expected {n} action values, got {len(a)}")
        if not all(math.isfinite(x) for x in a):raise ValueError("Actions must be finite")
        return [max(-1.,min(1.,x)) for x in a]
    def step(self,action):
        if self._closed:raise RuntimeError("Environment is closed; call reset() before stepping")
        if self._native is not None:
            r=self._native.step(list(action));self._steps+=1
            return list(r.observation),float(r.reward),bool(r.terminated),bool(r.truncated),dict(r.info)
        self._steps+=1;info={"step_count":self._steps};terminated=False
        if self.kind=="Navigate":
            a=self._actions(action,2);x,y,tx,ty=self._state;old=math.hypot(tx-x,ty-y);x=max(0.,min(10.,x+a[0]*.15));y=max(0.,min(10.,y+a[1]*.15));self._state=[x,y,tx,ty];d=math.hypot(tx-x,ty-y);terminated=d<.35;reward=old-d-.01+(1. if terminated else 0.);info["distance_to_target"]=d
        elif self.kind=="PoleBalance":
            a=self._actions(action,1);ang,av,cart,cv=self._state;force=a[0]*10;dt=1/60;aa=9.81*math.sin(ang)-.08*av+.025*force*math.cos(ang);ca=(force-.1*cv+.4*math.sin(ang))/1.1;av+=aa*dt;ang+=av*dt;cv+=ca*dt;cart+=cv*dt;self._state=[ang,av,cart,cv];terminated=abs(ang)>.45 or abs(cart)>2.4;reward=0. if terminated else 1.;info["angle"]=ang
        elif self.kind=="StackBlocks":
            a=self._actions(action,1);self._placement_x=max(0.,min(10.,self._placement_x+a[0]*.25));aligned=abs(self._placement_x-self._top_x)<.4
            if aligned:self._height+=1;self._top_x=self._placement_x;self._placement_x=max(0.,min(10.,self._top_x+self._rng.uniform(-.6,.6)))
            terminated=self._height>=4 or not aligned;reward=(.2 if aligned else -.2)+(1. if self._height>=4 else 0.);info.update({"height":self._height,"placed":aligned})
        else:
            a=self._actions(action,2);s=self._state;s[0]=max(.3,min(9.7,s[0]+a[0]*.08));s[1]=max(.3,min(5.7,s[1]+a[1]*.08))
            if abs(s[0]-s[4])<.8 and abs(s[1]-s[5])<.8:s[6]+=a[0]*.04;s[7]+=a[1]*.04
            s[4]=max(.55,min(9.45,s[4]+s[6]));s[5]=max(.55,min(5.45,s[5]+s[7]));d=math.hypot(s[8]-s[4],s[9]-s[5]);terminated=d<.65;reward=-d*.01-.001+(1. if terminated else 0.);info["distance_to_target"]=d
        return self._observe(),reward,terminated,self._steps>=self.max_steps and not terminated,info
    def close(self):
        if self._native is not None:self._native.close()
        self._closed=True
def make(env_id,seed=None,headless=True,**kwargs):return Env(env_id,seed=seed,headless=headless,**kwargs)
