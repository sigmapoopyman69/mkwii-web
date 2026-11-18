#include "browser_guest_memory.h"
#include <array>
#include <algorithm>
#include <cstring>
#include <memory>
#include <new>
namespace BrowserGuestMemory {
namespace {
constexpr std::uint32_t kMem1Physical=0x00000000u, kMem1Cached=0x80000000u, kMem1Uncached=0xC0000000u;
constexpr std::uint32_t kMem2Physical=0x10000000u, kMem2Cached=0x90000000u, kMem2Uncached=0xD0000000u;
constexpr std::size_t kMem1Pages=Memory::kMem1Size/Memory::kPageSize;
constexpr std::size_t kMem2Pages=Memory::kMem2Size/Memory::kPageSize;
struct Location { unsigned region; std::uint32_t offset; };
bool Decode(std::uint32_t a, Location& o) noexcept {
 const std::array<std::uint32_t,3> m1{{kMem1Physical,kMem1Cached,kMem1Uncached}};
 const std::array<std::uint32_t,3> m2{{kMem2Physical,kMem2Cached,kMem2Uncached}};
 for(auto b:m1) if(a>=b && static_cast<std::uint64_t>(a)-b<Memory::kMem1Size){o={0,a-b};return true;}
 for(auto b:m2) if(a>=b && static_cast<std::uint64_t>(a)-b<Memory::kMem2Size){o={1,a-b};return true;}
 return false;
}
}
struct Memory::Impl {
 using Page=std::array<std::uint8_t,kPageSize>;
 std::array<std::unique_ptr<Page>,kMem1Pages> mem1{};
 std::array<std::unique_ptr<Page>,kMem2Pages> mem2{};
 const Page* Find(unsigned r,std::uint32_t o) const noexcept {auto p=o/kPageSize;return r==0?mem1[p].get():mem2[p].get();}
 Page* Ensure(unsigned r,std::uint32_t o) noexcept {auto p=o/kPageSize;auto& s=r==0?mem1[p]:mem2[p];if(!s)s.reset(new(std::nothrow) Page{});return s.get();}
};
Memory::Memory():impl_(new Impl()){}
Memory::~Memory()=default;
void Memory::Reset() noexcept {for(auto& p:impl_->mem1)p.reset();for(auto& p:impl_->mem2)p.reset();}
bool Memory::Contains(std::uint32_t a,std::size_t n) const noexcept {
 if(n==0)return true;
 if(static_cast<std::uint64_t>(a)+n>0x100000000ull)return false;
 Location l{};
 if(!Decode(a,l))return false;
 const std::uint64_t regionSize=l.region==0?kMem1Size:kMem2Size;
 return static_cast<std::uint64_t>(l.offset)+n<=regionSize;
}
bool Memory::Read8(std::uint32_t a,std::uint8_t& v) const noexcept {Location l{};if(!Decode(a,l))return false;auto p=impl_->Find(l.region,l.offset);v=p?(*p)[l.offset%kPageSize]:0;return true;}
bool Memory::Read16(std::uint32_t a,std::uint16_t& v) const noexcept {if(!Contains(a,2))return false;std::uint8_t x=0,y=0;if(!Read8(a,x)||!Read8(a+1,y))return false;v=(std::uint16_t(x)<<8)|y;return true;}
bool Memory::Read32(std::uint32_t a,std::uint32_t& v) const noexcept {if(!Contains(a,4))return false;std::uint8_t x=0,y=0,z=0,w=0;if(!Read8(a,x)||!Read8(a+1,y)||!Read8(a+2,z)||!Read8(a+3,w))return false;v=(std::uint32_t(x)<<24)|(std::uint32_t(y)<<16)|(std::uint32_t(z)<<8)|w;return true;}
bool Memory::ReadBytes(std::uint32_t a,void* destination,std::size_t length) const noexcept {
 if(length==0)return true;
 if(destination==nullptr||!Contains(a,length))return false;
 auto* out=static_cast<std::uint8_t*>(destination);
 std::size_t remaining=length;
 std::uint32_t address=a;
 while(remaining){
  Location l{};
  if(!Decode(address,l))return false;
  const std::size_t pageOffset=l.offset%kPageSize;
  const std::size_t chunk=std::min(remaining,std::size_t(kPageSize)-pageOffset);
  const Page* page=impl_->Find(l.region,l.offset);
  if(page)std::memcpy(out,page->data()+pageOffset,chunk);
  else std::memset(out,0,chunk);
  out+=chunk;address+=static_cast<std::uint32_t>(chunk);remaining-=chunk;
 }
 return true;
}
bool Memory::Write8(std::uint32_t a,std::uint8_t v) noexcept {Location l{};if(!Decode(a,l))return false;auto p=impl_->Ensure(l.region,l.offset);if(!p)return false;(*p)[l.offset%kPageSize]=v;return true;}
bool Memory::Write16(std::uint32_t a,std::uint16_t v) noexcept {if(!Contains(a,2))return false;return Write8(a,std::uint8_t(v>>8))&&Write8(a+1,std::uint8_t(v));}
bool Memory::Write32(std::uint32_t a,std::uint32_t v) noexcept {if(!Contains(a,4))return false;return Write8(a,std::uint8_t(v>>24))&&Write8(a+1,std::uint8_t(v>>16))&&Write8(a+2,std::uint8_t(v>>8))&&Write8(a+3,std::uint8_t(v));}
bool Memory::WriteBytes(std::uint32_t a,const void* source,std::size_t length) noexcept {
 if(length==0)return true;
 if(source==nullptr||!Contains(a,length))return false;
 const auto* in=static_cast<const std::uint8_t*>(source);
 std::size_t remaining=length;
 std::uint32_t address=a;
 while(remaining){
  Location l{};
  if(!Decode(address,l))return false;
  const std::size_t pageOffset=l.offset%kPageSize;
  const std::size_t chunk=std::min(remaining,std::size_t(kPageSize)-pageOffset);
  Page* page=impl_->Ensure(l.region,l.offset);
  if(!page)return false;
  std::memcpy(page->data()+pageOffset,in,chunk);
  in+=chunk;address+=static_cast<std::uint32_t>(chunk);remaining-=chunk;
 }
 return true;
}
std::size_t Memory::AllocatedPageCount() const noexcept {std::size_t n=0;for(const auto& p:impl_->mem1)if(p)++n;for(const auto& p:impl_->mem2)if(p)++n;return n;}
bool RunSmokeTest() noexcept {
 Memory m;std::uint32_t v=0;std::uint16_t h=0;
 if(!m.Write32(0x80001000u,0x1234ABCDu)||!m.Read32(0x00001000u,v)||v!=0x1234ABCDu)return false;
 if(!m.Read32(0xC0001000u,v)||v!=0x1234ABCDu)return false;
 if(!m.Write16(0x90002000u,0xBEEFu)||!m.Read16(0x10002000u,h)||h!=0xBEEFu)return false;
 if(m.Contains(0x81800000u)||m.Contains(0x98000000u)||m.Write8(0x70000000u,1))return false;
 if(m.AllocatedPageCount()!=2)return false;m.Reset();if(m.AllocatedPageCount()!=0)return false;
 return m.Read32(0x80001000u,v)&&v==0;
}
}
