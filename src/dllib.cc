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





#ifdef _WIN32


std::string dll_file::locate()
{
    iostr  file = "%s.dll";
    file << _id;
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
    FindClose(handle);
    return file();
}

rem::code dll_file::open()
{
    std::string str_location = locate();
    if (str_location.empty()) {
        //return { (
        //    Annotation::Push(),
        //    Annotation::Type::Error,' ',
        //    "Library file '", _ID, "' ",
        //    Annotation::Code::NotFound,
        //    " Within the system paths."
        //) };
        //throw  message::push(message::t::error), id, " not found into valid paths.";
        return rem::code::null_ptr;
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
        // ---------------------------------------------------------------------------------------
        //return { (
        //    Annotation::Push(),
        //    Annotation::Type::Error,
        //    Annotation::Code::Null,
        //    "Openning ", _ID, "Library file: [",SysMsg, "]."
        //) };
        std::cerr << SysMsg << "\n";
        return rem::code::empty;
    }
    //throw  message::push(message::t::error), id, ": dll error";

//GetProcAddress;
    FARPROC _export_fn = nullptr;
    _export_fn = GetProcAddress(_Handle, EXPORT_SYM);
    if (!_export_fn) {
        FreeLibrary(_Handle);
    LeaveWithNoInterface:
        //return { (
        //    Annotation::Push(),
        //    Annotation::Type::Error,
        //    Annotation::Code::Empty,
        //    " - rtdl ", _ID, ": does not - or failed to - provides interface."
        //) };
        //throw message::push(message::t::error), " rtdl ", id, ": rtdl does not provides exported interface.";
        return rem::code::null_ptr;
    }

    _interface = reinterpret_cast<rtdl::interface_map(*)()>(_export_fn)();
    if (_interface.empty()) {
        FreeLibrary(_Handle);
        goto LeaveWithNoInterface;
    }

    for (auto& ix : _interface) {
        ix.second = GetProcAddress(_Handle, ix.first.c_str());
        if (!ix.second) {
            FreeLibrary(_Handle);
            //return { (
            //    Annotation::Push(),
            //    Annotation::Type::Error,
            //    Annotation::Code::Null,
            //    "- rtdl '", _ID, " has no `exported` symbol identified by '", ix.first,"', while listed in its exported interface."
            //) };
            //throw message::push(message::t::error), " rtdl ", id, ": exported symbol is unbound: [", ix.first, "]";
            return rem::code::empty;
        }
    }
    if ((_rtdl = (reinterpret_cast<rtdl * (*)()>(_interface[CREATE_SYM]))()) != nullptr) {
        _rtdl->set_interface(_interface);
        _interface.clear(); // Do not keep a local copy since the interface data is now owned by the rtdl Instance...
    }
    return rem::code::ok;
}
