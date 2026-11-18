#include "browser_runtime_memory.h"

#include <array>
#include <cstring>

namespace BrowserRuntimeMemory {

bool Adapter::Contains(std::uint32_t a,std::size_t n) const noexcept { return memory_.Contains(a,n); }
bool Adapter::Read8(std::uint32_t a,std::uint8_t& v) const noexcept { return memory_.Read8(a,v); }
bool Adapter::Read16(std::uint32_t a,std::uint16_t& v) const noexcept { return memory_.Read16(a,v); }
bool Adapter::Read32(std::uint32_t a,std::uint32_t& v) const noexcept { return memory_.Read32(a,v); }
bool Adapter::Read64(std::uint32_t a,std::uint64_t& v) const noexcept {
    std::uint8_t b[8]{};
    if(!memory_.ReadBytes(a,b,sizeof(b)))return false;
    std::uint64_t result=0;
    for(std::uint8_t byte:b)result=(result<<8)|byte;
    v=result;
    return true;
}
bool Adapter::ReadFloat32(std::uint32_t a,float& v) const noexcept {
    std::uint32_t bits=0;
    if(!Read32(a,bits))return false;
    std::memcpy(&v,&bits,sizeof(v));
    return true;
}
bool Adapter::ReadFloat64(std::uint32_t a,double& v) const noexcept {
    std::uint64_t bits=0;
    if(!Read64(a,bits))return false;
    std::memcpy(&v,&bits,sizeof(v));
    return true;
}
bool Adapter::Write8(std::uint32_t a,std::uint8_t v) noexcept { return memory_.Write8(a,v); }
bool Adapter::Write16(std::uint32_t a,std::uint16_t v) noexcept { return memory_.Write16(a,v); }
bool Adapter::Write32(std::uint32_t a,std::uint32_t v) noexcept { return memory_.Write32(a,v); }
bool Adapter::Write64(std::uint32_t a,std::uint64_t v) noexcept {
    std::uint8_t b[8]{};
    for(int i=7;i>=0;--i){b[i]=static_cast<std::uint8_t>(v);v>>=8;}
    return memory_.WriteBytes(a,b,sizeof(b));
}
bool Adapter::WriteFloat32(std::uint32_t a,float v) noexcept {
    std::uint32_t bits=0;
    std::memcpy(&bits,&v,sizeof(bits));
    return Write32(a,bits);
}
bool Adapter::WriteFloat64(std::uint32_t a,double v) noexcept {
    std::uint64_t bits=0;
    std::memcpy(&bits,&v,sizeof(bits));
    return Write64(a,bits);
}
bool Adapter::CopyFromGuest(std::uint32_t a,void* d,std::size_t n) const noexcept {
    return memory_.ReadBytes(a,d,n);
}
bool Adapter::CopyToGuest(std::uint32_t a,const void* s,std::size_t n) noexcept {
    return memory_.WriteBytes(a,s,n);
}

bool RunSmokeTest() noexcept {
    BrowserGuestMemory::Memory backing;
    Adapter memory(backing);
    std::uint64_t wide=0;
    float single=0;
    double real=0;
    constexpr std::uint64_t kWide=0x0123456789ABCDEFull;
    constexpr float kSingle=1.25f;
    constexpr double kDouble=-123.5;
    if(!memory.Write64(0x80002FFCu,kWide))return false;
    if(!memory.Read64(0x00002FFCu,wide)||wide!=kWide)return false;
    if(!memory.WriteFloat32(0x80004000u,kSingle)||
       !memory.ReadFloat32(0xC0004000u,single)||single!=kSingle)return false;
    if(!memory.WriteFloat64(0x90005000u,kDouble)||
       !memory.ReadFloat64(0x10005000u,real)||real!=kDouble)return false;

    const std::array<std::uint8_t,12> source{{0,1,2,3,4,5,6,7,8,9,10,11}};
    std::array<std::uint8_t,12> copied{};
    if(!memory.CopyToGuest(0x80006FFAu,source.data(),source.size()))return false;
    if(!memory.CopyFromGuest(0xC0006FFAu,copied.data(),copied.size())||copied!=source)return false;
    if(memory.Contains(0x817FFFFCu,8)||memory.Write64(0x817FFFFCu,kWide))return false;
    if(memory.Read64(0x70000000u,wide))return false;
    return true;
}

} // namespace BrowserRuntimeMemory
