Name:           koverlay
Version:        1.0.0
Release:        1%{?dist}
Summary:        Click-through overlay for Wayland (Qt 6 + LayerShellQt)

License:        MIT
URL:            https://github.com/erx/koverlay
# Point to a release tarball or use git archive (see below)
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  make
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  qt6-qtwayland-devel
BuildRequires:  layer-shell-qt-devel
BuildRequires:  wayland-devel
BuildRequires:  pkgconfig(xkbcommon)

# Helpful runtime hints (autodeps will also catch .so):
Requires:       layer-shell-qt
Requires:       qt6-qtbase
Requires:       qt6-qtdeclarative
Requires:       qt6-qtwayland

%description
KOverlay is a click-through, always-on-top overlay for KDE/Wayland (Qt 6 + QML + LayerShellQt).
Useful for sticky notes, cheat sheets, and keybindings. Supports hot-reloading config.

%prep
%autosetup -n %{name}-%{version}

%build
%cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo
%cmake_build

%install
%cmake_install


%files
%license LICENSE
%doc README.md
%{_bindir}/koverlay
%{_datadir}/applications/koverlay.desktop

%changelog
* Sat Oct 04 2025 Erik - 1.0.0-1
- Initial build
