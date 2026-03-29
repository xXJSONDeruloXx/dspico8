pico-8 cartridge // http://www.pico-8.com
version 42
__lua__
t=0

function _init()
end

function _update60()
 t=t+1
end

function _draw()
 cls(0)
 for y=0,15 do
  for x=0,15 do
   rectfill(x*8,y*8,x*8+7,y*8+7,(x+y+t)%16)
  end
 end
 for i=0,127 do
  pset(i,(i+t)%128,7)
 end
end
