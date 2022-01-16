cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(czatui CXX)

add_executable(ui
    appsettings.cpp autologindatadialog.cpp filebasedlogger.cpp main.cpp
    mainwindow.cpp captchadialog.cpp mainchatwindow.cpp chatwindowtabwidget.cpp
    notificationsupport.cpp notificationsupport_msgbox.cpp settingsbasedblocker.cpp
    settingsdialog.cpp userlistview.cpp

    autologindatadialog.ui chatsettingsform.ui chatwidget.ui mainwindow.ui
    captchadialog.ui settingsdialog.ui

    rsrc.qrc
)

if(WIN32)
    target_sources(ui PRIVATE
        notificationsupport_win10.cpp ../WinToast/src/wintoastlib.cpp
        icons/windows.rc)
    target_compile_definitions(ui PRIVATE UNICODE _UNICODE PSAPI_VERSION=1)
    set_target_properties(ui PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(${${qt_ver_pfx}DBus_FOUND})
    target_sources(ui PRIVATE notificationsupport_dbus.cpp)
    set(extra_libs ${qt_ver_pfx}::DBus)
endif()

target_link_libraries(ui PRIVATE czatlib ${qt_ver_pfx}::Widgets ${extra_libs} ${UI_EXTRA_QT_LIBS})
target_include_directories(ui PRIVATE ${CMAKE_SOURCE_DIR})
