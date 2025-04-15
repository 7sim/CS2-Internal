#include "csgoinput.h"

#include "../../utils/utils.h"
#include "csgoinput.h"
#include "..\..\sdk\sdk.h"
#include "..\..\sdk\interfaces\memory.h"
#include "..\..\utils\hook\hooks.h"
CMsgQAngle* CCSGOInputHistoryEntryPB::create_new_angle()
{
    this->nCachedBits |= 1;

    auto a1 = (DWORD64)(nHasBits & 0xFFFFFFFFFFFFFFFC);

    if ((nHasBits & 1) != 0)
        a1 = (DWORD64)a1;
    using fn = CMsgQAngle * (CS_FASTCALL)(DWORD64);

    return hooks::m_create_cmsg_qangle(a1);
}
CMsgVector * CCSGOInputHistoryEntryPB::create_new_vector_angle()
{
    this->nCachedBits |= 1;

    auto a1 = (DWORD64)(nHasBits & 0xFFFFFFFFFFFFFFFC);

    if ((nHasBits & 1) != 0)
        a1 = (DWORD64)a1;
    using fn = CMsgVector * (CS_FASTCALL)(DWORD64);

    return hooks::m_create_cmsg_vector(a1);
}

CCSGOInterpolationInfoPB* CCSGOInputHistoryEntryPB::create_new_interp()
{
    this->nCachedBits |= 1;

    auto a1 = (DWORD64)(nHasBits & 0xFFFFFFFFFFFFFFFC);

    if ((nHasBits & 1) != 0)
        a1 = (DWORD64)a1;

    using fn = CCSGOInterpolationInfoPB * (CS_FASTCALL)(DWORD64);

    return hooks::m_create_ccsgointerpolation_info(a1);


}
CCSGOInterpolationInfoPB_CL* CCSGOInputHistoryEntryPB::create_new_interp_cl()
{
    this->nCachedBits |= 1;

    auto a1 = (DWORD64)(nHasBits & 0xFFFFFFFFFFFFFFFC);

    if ((nHasBits & 1) != 0)
        a1 = (DWORD64)a1;

    using fn = CCSGOInterpolationInfoPB_CL * (CS_FASTCALL)(DWORD64);

    return hooks::m_create_ccsgointerpolation_info_cl(a1);


}
CSubtickMoveStep* CUserCmd::create_subtick_step(void* arena) {

    return hooks::m_create_sub_tick_step_move(arena);
}


CSubtickMoveStep* CUserCmd::create_subtick() {
    if (!this->pBaseCmd)
        return nullptr;

    auto base = this->pBaseCmd;

    if (base->subtickMovesField.pRep && base->subtickMovesField.nCurrentSize < base->subtickMovesField.pRep->nAllocatedSize) {
        return base->subtickMovesField.pRep->tElements[base->subtickMovesField.nCurrentSize++];
    }

    auto new_subtick = hooks::m_create_sub_tick_step_move(base->subtickMovesField.pArena);

    if (!new_subtick)
        return nullptr;
  
    auto sub_step = hooks::m_create_sub_tick(base->subtickMovesField, new_subtick);
    if (!sub_step)
        return nullptr;

    return sub_step;
}



CSubtickMoveStep* CBaseUserCmdPB::create_new_subtick_step() {
    auto base = this;

    if (!base)
        return nullptr;

    if (base->subtickMovesField.pRep && base->subtickMovesField.nCurrentSize < base->subtickMovesField.pRep->nAllocatedSize)
        return base->subtickMovesField.pRep->tElements[base->subtickMovesField.nCurrentSize++];
    
    auto new_subtick = hooks::m_create_sub_tick_step_move(base->subtickMovesField.pArena);
    if (!new_subtick)
        return nullptr;

    auto sub_step = hooks::m_create_sub_tick(base->subtickMovesField, new_subtick);
    if (!sub_step)
        return nullptr;

    return sub_step;
}
#define DWORD2(x) (*(reinterpret_cast<uintptr_t*>(&x) + 2))
static uint8_t old_crc_size = 0;
bool CUserCmd::CRC(CBaseUserCmdPB* m_base) {
    if (!this || !m_base || hooks::m_serialize == nullptr)
        return false;

    if (!g::m_active_weapon || !g::m_local_player_pawn)
        return false;

    auto size = m_base->get_crc_size();
    std::string* prevChecksum = m_base->strMoveCrc;

    CUtlBuffer m_buffer = CUtlBuffer::CUtlBuffer(0,0,0);
    m_buffer.ensure(size + 1);


    if (hooks::m_serialize(m_base, m_buffer, static_cast<int>(size))) {
        std::uintptr_t* m_message = reinterpret_cast<uintptr_t*>(sdk::m_mem_alloc->Alloc(0x18));

        m_base->nCachedBits |= 1u;
        auto nHasBits = static_cast<uint32_t>(m_base->nHasBits & 0xFFFFFFFFFFFFFFFC);
        if ((m_base->nHasBits & 1) != 0)
            nHasBits = static_cast<uint32_t>(nHasBits);


        hooks::m_parse(m_message, m_buffer, size);

        m_base->strMoveCrc = hooks::m_merge(&m_base->strMoveCrc, m_message, &nHasBits);

        sdk::m_mem_alloc->Free(m_message);

        return true;
    }
    
   return false;
    /* pseudo game code
     CUtlBuffer::CUtlBuffer((CUtlBuffer *)&buffer, 0, 0, 0);
      v159 = (_DWORD *)getCrcSize((__int64)&pBaseCmd);
      size = (unsigned int)v159;
      CUtlBuffer::EnsureCapacity((CUtlBuffer *)&buffer, (_DWORD)v159 + 1);
      if ( SERIALIZE((__int64)&pBaseCmd, (__int64)buffer, size) )
      {
        *((_DWORD *)v3 + 4) |= 1u;
        if ( !v3[6] )
        {
          v90 = (_QWORD *)(*v74 & 0xFFFFFFFFFFFFFFFCui64);
          if ( (*(_BYTE *)v74 & 1) != 0 )
            v90 = (_QWORD *)*v90;
          v3[6] = sub_180259420((__int64)v90);
        }
        basecmd = v3[6];
        freshBuffer = (__int64)buffer;
        *(_DWORD *)(basecmd + 0x10) |= 1u;
        szMoveCrc = basecmd + 0x30;
        bits = (_QWORD *)(*(_QWORD *)(basecmd + 8) & 0xFFFFFFFFFFFFFFFCui64);
        if ( (*(_BYTE *)(basecmd + 8) & 1) != 0 )
          bits = (_QWORD *)*bits;
        message = 0i64;
        v135 = 0i64;
        v136 = 0i64;
        sub_1800F73F0((unsigned __int64 *)&message, freshBuffer, (int)v159);
        sub_180CC0990(szMoveCrc, &message, bits);
        if ( v136 >= 0x10 )
        {
          if ( v136 + 1 >= 0x1000 && (unsigned __int64)(message - *(_QWORD *)(message - 8) - 8) > 0x1F )
            invalid_parameter_noinfo_noreturn();
          sub_1808128D0();
        }
  }
    
    */
}
void CCSGOInput::add_button(ECommandButtons button) {
    /* to check if last frame button was down:
      prev_button = *((_QWORD *)current_button - 1);
    if ( prev_button == 1 || prev_button == 2048 )
    {
    */
    if (!this)
        return;

    const auto button_state{ hooks::m_get_button_state(this, 0) };

    auto& buttonCount{ recast_at_pointer(int, button_state, 0x34) };

    auto currentButton{ (BYTE*)(reinterpret_cast<std::uintptr_t>(button_state) + 24 * (3 + buttonCount)) };

    auto& v15{ recast_at_pointer(__int64, currentButton, -8) };
    v15 = button;
    *currentButton |= button;

    buttonCount += 1;
}
void c_input_handler::Poll(CCSGOInput* input) {
    

}
CUserCmd* CUserCmd::Get()
{
    auto controller = CCSPlayerController::Get();
    if (!controller)
        return nullptr;

    static void* (__fastcall * GetCommandIndex)(void* pController, int* index) = reinterpret_cast<decltype(GetCommandIndex)>
        (utils::GetAbsoluteAddress(utils::FindPattern("client.dll", ("E8 ? ? ? ? 8B 8D ? ? ? ? 8D 51")), 0x1, 0x0));
    debug(GetCommandIndex != nullptr);

    if (!GetCommandIndex)
        return nullptr;

    int index = 0;
    GetCommandIndex(controller, &index);
    int commandIndex = index - 1;

    if (commandIndex == -1)
        commandIndex = 0xFFFFFFFF;

    static void* (__fastcall * GetUserCmdBase)(void* a1, int a2) = reinterpret_cast<decltype(GetUserCmdBase)>
        (utils::GetAbsoluteAddress(utils::FindPattern("client.dll", ("E8 ? ? ? ? 48 8B CF 4C 8B E8 44 8B B8")), 0x1, 0x0));
    debug(GetUserCmdBase != nullptr);

    if (!GetUserCmdBase)
        return nullptr;

    static void* off_181D9AAA0 = *reinterpret_cast<void**>(utils::ResolveRelativeAddress(utils::FindPattern("client.dll", ("48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8B CF 4C 8B E8")), 0x3, 0x7));

    if (!off_181D9AAA0)
        return nullptr;

    void* pUserCmdBase = GetUserCmdBase(off_181D9AAA0, commandIndex);
    if (!pUserCmdBase)
        return nullptr;

    DWORD sequenceNumber = *reinterpret_cast<DWORD*>((uintptr_t)pUserCmdBase + 0x5C00);
    static CUserCmd* (__fastcall * GetUserCmd)(void* pController, DWORD sequenceNumber) = reinterpret_cast<decltype(GetUserCmd)>
        (utils::GetAbsoluteAddress(utils::FindPattern("client.dll", ("E8 ? ? ? ? 48 8B 0D ? ? ? ? 45 33 E4 48 89 44 24")), 0x1, 0x0));

    if (!GetUserCmd)
        return nullptr;

    CUserCmd* pUserCmd = GetUserCmd(controller, sequenceNumber);
    if (!pUserCmd)
        return nullptr;

    return pUserCmd;
}

CBaseUserCmdPB* CUserCmd::GetBase()
{
    auto controller = CCSPlayerController::Get();
    if (!controller)
        return nullptr;

    static void* (__fastcall * GetCommandIndex)(void* pController, int* index) = reinterpret_cast<decltype(GetCommandIndex)>
        (utils::GetAbsoluteAddress(utils::FindPattern("client.dll", ("E8 ? ? ? ? 8B 8D ? ? ? ? 8D 51")), 0x1, 0x0));
    debug(GetCommandIndex != nullptr);

    if (!GetCommandIndex)
        return nullptr;

    int index = 0;
    GetCommandIndex(controller, &index);
    int commandIndex = index - 1;

    if (commandIndex == -1)
        commandIndex = 0xFFFFFFFF;

    static CBaseUserCmdPB* (__fastcall * GetUserCmdBase)(void* a1, int a2) = reinterpret_cast<decltype(GetUserCmdBase)>
        (utils::GetAbsoluteAddress(utils::FindPattern("client.dll", ("E8 ? ? ? ? 48 8B CF 4C 8B E8 44 8B B8")), 0x1, 0x0));
    debug(GetUserCmdBase != nullptr);

    if (!GetUserCmdBase)
        return nullptr;

    static void* off_181D9AAA0 = *reinterpret_cast<void**>(utils::ResolveRelativeAddress(utils::FindPattern("client.dll", ("48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8B CF 4C 8B E8")), 0x3, 0x7));

    if (!off_181D9AAA0)
        return nullptr;

    CBaseUserCmdPB*   pUserCmdBase = GetUserCmdBase(off_181D9AAA0, commandIndex);
    if (!pUserCmdBase)
        return nullptr;

    
    return pUserCmdBase;
}

void c_input_handler::Press(uint64_t button, float when) {
    if (!move) return;

    // we always need to input 2 subtick inputs
     if (move->nAdditionalStepMovesCount > 10) {
         _log(LOG_WARNING) << "[CMoveStepData] Too many subticks already queued, input ignored";
         return;
    }

    if (move->nAdditionalStepMovesCount > 0) when = move->tinyMoveStepData[move->nAdditionalStepMovesCount - 1].flWhen;

    CTinyMoveStepData& pressInput = move->tinyMoveStepData[move->nAdditionalStepMovesCount++];
    CTinyMoveStepData& releaseInput = move->tinyMoveStepData[move->nAdditionalStepMovesCount++];

    pressInput.nButton = releaseInput.nButton = button;

    if (false && IsHeld(button)) {
        // interupt hold
        pressInput.bPressed = false;
        pressInput.flWhen = when + 0.05f;
        // press it back down
        releaseInput.bPressed = true;
        releaseInput.flWhen = when + 0.1f;
    }
    else {
        // press it
        pressInput.bPressed = true;
        pressInput.flWhen = when;
        // release it
        releaseInput.bPressed = false;
        releaseInput.flWhen = when;
    }
}

bool c_input_handler::Release(uint64_t button) {
    if (!move) return false;

    bool removed = false;
    for (uint32_t i = 0; i < move->nAdditionalStepMovesCount; ++i) {
        CTinyMoveStepData& subInput = move->tinyMoveStepData[i];
        if (subInput.nButton != button) continue;
        if (subInput.bPressed) {
            if (move->nAdditionalStepMovesCount - i > 0)
                memmove(&subInput, &move->tinyMoveStepData[i + 1], sizeof(CTinyMoveStepData) * (move->nAdditionalStepMovesCount - i));
            move->nAdditionalStepMovesCount--;
            i--;
            removed = true;
        }
    }

    /* move->nAdditionalStepMovesCount - i > 0 */

    if (!IsReleased(button)) {
        if (move->nAdditionalStepMovesCount >= 12) {
           _log(LOG_WARNING) << "[CMoveStepData] Too many subticks already queued, input ignored";
            return false;
        }

        CTinyMoveStepData& subInput = move->tinyMoveStepData[move->nAdditionalStepMovesCount++];
        subInput.nButton = button;
        subInput.bPressed = false;
        subInput.flWhen = 1.f;
    }

    return removed;
}

void c_input_handler::Dump() {
    if (!move) return;
    if (move->nAdditionalStepMovesCount < 1) return;
    _log(LOG_INFO) << "================================================";

    _log(LOG_INFO) << "Subtick count: " << move->nAdditionalStepMovesCount;

    for (uint32_t i = 0; i < move->nAdditionalStepMovesCount; ++i) {
        CTinyMoveStepData& subInput = move->tinyMoveStepData[i];
        _log(LOG_INFO) << "[CMoveStepData] current tick index:"<<  i << "button: " << subInput.nButton << " held:" << subInput.bPressed << "when: " << subInput.flWhen;
    }
}