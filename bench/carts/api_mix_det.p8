pico-8 cartridge // http://www.pico-8.com
version 42
__lua__
t=0

function _init()
 for i=0,15 do
  fset(i,i%8,true)
  mset(i,0,i)
  mset(i,1,15-i)
  for px=0,7 do
   sset(i*8+px,i%8,(i+px)%16)
  end
 end
end

function _update60()
 t=t+1
 camera((t%5)-2,(t%7)-3)
 clip(4,4,120,120)
 mset(t%16,1,(15-t)%16)
end

function _draw()
 cls(1)
 map(0,0,0,0,16,2)
 for i=0,15 do
  if fget(i,i%8) then
   spr(i,(i*8+t)%128,32+(i%4)*8)
  end
 end
 rect(10,10,50,30,8)
 rectfill(52,12,92,28,9)
 line(0,127,127,0,7)
 camera()
 clip()
 pset(64,64,6)
end
