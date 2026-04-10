Name:           musicassistant-native
Version:        2026.04.10
Release:        1%{?dist}
Summary:        A native KDE client for Music Assistant

License:        GPL-3.0-or-later
URL:            https://github.com/music-assistant

Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.20
BuildRequires:  gcc-c++
BuildRequires:  extra-cmake-modules >= 6.0
BuildRequires:  cmake(Qt6Core)
BuildRequires:  cmake(Qt6Quick)
BuildRequires:  cmake(Qt6QuickControls2)
BuildRequires:  cmake(Qt6WebSockets)
BuildRequires:  cmake(Qt6Multimedia)
BuildRequires:  cmake(Qt6Network)
BuildRequires:  cmake(Qt6Widgets)
BuildRequires:  cmake(Qt6Qml)
BuildRequires:  cmake(Qt6DBus)
BuildRequires:  cmake(KF6Kirigami)
BuildRequires:  cmake(KF6I18n)
BuildRequires:  cmake(KF6CoreAddons)
BuildRequires:  cmake(KF6Config)
BuildRequires:  cmake(KF6DBusAddons)
BuildRequires:  cmake(KF6Notifications)
BuildRequires:  cmake(KF6WindowSystem)
BuildRequires:  cmake(KF6IconThemes)
BuildRequires:  pkgconfig(flac)
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib

Requires:       kf6-kirigami
Requires:       qt6-qtwebsockets
Requires:       qt6-qtdeclarative
Requires:       flac-libs

%description
Music Assistant Native is a native KDE Plasma desktop application for controlling
Music Assistant, the open-source music library manager. Connect to your Music
Assistant server and browse your unified music library, control playback on any
connected speaker, and manage your play queue with a Plasma-native interface.

%prep
%autosetup -n %{name}-%{version}

%build
%cmake -DCMAKE_BUILD_TYPE=Release
%cmake_build

%install
%cmake_install

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/io.github.musicassistant.native.desktop

%files
%license COPYING
%doc CLAUDE.md
%{_bindir}/musicassistant-native
%{_datadir}/applications/io.github.musicassistant.native.desktop
%{_datadir}/metainfo/io.github.musicassistant.native.metainfo.xml
%{_datadir}/icons/hicolor/scalable/apps/musicassistant-native.svg
