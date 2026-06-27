/////////////////////////////////////////////////////////////////////////////
// Purpose:   Language keyword strings for scintilla syntax highlighting
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

inline constexpr const char* g_u8_cpp_keywords =
    "alignas alignof and and_eq atomic_cancel atomic_commit atomic_noexcept auto"
    " bitand bitor bool break case catch char char8_t char16_t char32_t"
    " class compl concept const consteval constexpr constinit const_cast"
    " continue co_await co_return co_yield __declspec"
    " decltype default delete dllexport do double dynamic_cast else enum explicit"
    " export extern false float for friend goto if inline int interface long"
    " mutable namespace new noexcept not not_eq nullptr operator private or or_eq"
    " private protected public reflexpr register reinterpret_cast requires"
    " return short signed sizeof static static_assert static_cast"
    " struct switch synchronized template this thread_local throw true try typedef typeid"
    " typename union unsigned using virtual void volatile wchar_t"
    " while xor xor_eq";

inline constexpr const char* g_python_keywords =
    "False None True and as assert async break class continue def del elif else except finally "
    "for from global if import in is lambda "
    "nonlocal not or pass raise return self try while with yield";

inline constexpr const char* g_ruby_keywords =
    "ENCODING LINE FILE BEGIN END alias and begin break case class def defined do else"
    " elsif end ensure false for if in module next nil not or redo require rescue retry"
    " return self super then true undef unless until when while yield";

inline constexpr const char* g_fortran_keywords =
    "abstract allocatable assign assignment associate asynchronous backspace bind block blockdata"
    " call case class close common complex contains continue contiguous critical cycle data"
    " deallocate default deferred dimension do double doubleprecision else elseif elsewhere"
    " end endblock endblockdata endcritical enddo endfile endforall endfunction endif endinterface"
    " endmodule endprocedure endprogram endselect endsubmodule endsubroutine endteam endtype"
    " endwhere entry equivalence error exit extends external final flush forall format function"
    " generic goto if images implicit import include inquire integer intent interface intrinsic"
    " lock logical module namelist non_overridable nopass nullify only open operator optional"
    " parameter pass pause pointer print private procedure program protected public pure read"
    " real recursive result return rewind save select sequence stop submodule subroutine sync"
    " target then type unlock use value volatile where write";

inline constexpr const char* g_go_keywords =
    "break case chan const continue default defer else fallthrough for func go goto if import"
    " interface map package range return select struct switch type var";

inline constexpr const char* g_julia_keywords =
    "abstract baremodule begin break catch ccall const continue do else elseif end export"
    " false finally for function global if import importall in isa let local macro module"
    " mutable new primitive quote return struct true try type using where while";

inline constexpr const char* g_luajit_keywords =
    "and break do else elseif end false for function goto if in local nil not or repeat"
    " return then true until while";

inline constexpr const char* g_typescript_keywords =
    "abstract any as async await boolean break case catch class const continue debugger"
    " declare default delete do else enum export extends false finally for function if"
    " implements import in instanceof interface let module new null number package private"
    " protected public readonly return static string super switch symbol this throw true"
    " try type typeof undefined var void while with yield";
