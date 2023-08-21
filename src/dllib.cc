/******************************************************************************************
 *   Copyright (C) 1965/1987/2023 by Serge Lussier                                        *
 *   serge.lussier@oldlonecoder.club                                                      *
 *                                                                                        *
 *                                                                                        *
 *   Unless otherwise specified, all code in this project is written                      *
 *   by the author (Serge Lussier)                                                        *
 *   and no one else then not even {copilot, chatgpt, or any other AI calamities}         *
 *   ----------------------------------------------------------------------------------   *
 *   Copyrights from authors other than Serge Lussier also apply here                     *
 *   Open source FREE licences also apply to the code from the same author (Serge Lussier)*
 *   ----------------------------------------------------------------------------------   *
 ******************************************************************************************/


#include "dllibcc/dllib.h"


#ifdef _WIN32

namespace dll
{

dllib::dllib(object* parent_obj, const std::string& lib_id) : book::object(parent_obj, lib_id) {}

dllib::~dllib()
{
    _interface.clear();
}

    
std::string dll_file::locate()
{
    stracc  file = "%s.dll";
    file << id();
    WIN32_FIND_DATA FindFileData;
#ifdef _UNICODE
    if (HANDLE handle = FindFirstFile((LPCWSTR)file().c_str(), &FindFileData); handle != INVALID_HANDLE_VALUE)
#else
#ifdef UNICODE
    if (HANDLE handle = FindFirstFile((LPCWSTR)file().c_str(), &FindFileData); handle != INVALID_HANDLE_VALUE)
#else
    if (HANDLE handle = FindFirstFile((LPCSTR)file().c_str(), &FindFileData); handle != INVALID_HANDLE_VALUE)
#endif
#endif
    {
        FindClose(handle);
        return file();
    }

    return file();
}



dll_file::dll_file(book::object* parent_obj, const std::string& dl_id): book::object(parent_obj, dl_id){}


book::rem::code dll_file::open()
{
    std::string str_location = locate();
    if (str_location.empty()) {
        book::rem::push_error(HERE) << " Library file " << str_location << book::rem::notexist << " in the system path.";
        return book::rem::notexist;
    }
#if defined(_UNICODE) || defined(UNICODE)
    _Handle = LoadLibrary((LPCWSTR)str_location.c_str());
#else
    _Handle = LoadLibrary((LPCSTR)str_location.c_str());
#endif
    if (!_Handle) {
        // ---------------------------------------------------------------------------------------
        //https://stackoverflow.com/a/17387176 --
        // Modified.
        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

        std::string SysMsg(messageBuffer, size);

        //Free the buffer.
        LocalFree(messageBuffer);
        book::rem::push_error() << " Library file '" << str_location << "' : "
            << book::rem::failed << " open/load: " << color::Yellow << SysMsg;
        return book::rem::empty;
    }

    // ---------- Import library's exported symbols : --------------------------------
    FARPROC _export_fn = nullptr;
    // --- Manually load the external _export function pointer from that library:
    _export_fn = GetProcAddress(_Handle, EXPORT_SYM);
    if (!_export_fn) {
        FreeLibrary(_Handle);
    LeaveWithNoInterface:
        return book::rem::null_ptr;
    }
    // -------------------------------------------------------------------------------

    // Then invoke the export function pointer that fills the interface_map:
    _interface = reinterpret_cast<dllib::interface_map(*)()>(_export_fn)();
    if (_interface.empty()) {
        FreeLibrary(_Handle);
        goto LeaveWithNoInterface;
    }

    for (auto& [sym_id, fn_ptr] : _interface) {
        fn_ptr = GetProcAddress(_Handle, sym_id.c_str());
        //ix.second = GetProcAddress(_Handle, ix.first.c_str());
        if (!fn_ptr) {
            FreeLibrary(_Handle);
            book::rem::push_error(HERE) << book::rem::aborted << " symbol '" << color::Yellow << sym_id << color::Reset << "' has no address.";
            return book::rem::empty;
        }
    }

    if ((_dl = (reinterpret_cast<dllib * (*)()>(_interface["_create"]))()) != nullptr) {
        _dl->set_interface(_interface);
        _interface.clear(); // Do not keep a local copy since the interface data is now owned by the rtdl Instance...
    }
    return book::rem::ok;
}



int dll_file::close()
{
    // _dl instance must be destroyed/released from the external library:
    // _dl->call("_del");
    (reinterpret_cast<void (*)(dllib*)>(_dl->_interface["_del"]))(_dl);
    FreeLibrary(_Handle);    
    return 0;
}

#endif 



}