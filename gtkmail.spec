%define version 1.2.0
%define release 1

Summary: gtkmail is a GUI email client for X
Name: gtkmail
Version: %{version}
Release: %{release}
Group: Application/Mail
Url: http://gtkmail.sourceforge.net/
Source: http://download.sourceforge.net/gtkmail/gtkmail-%{version}.tar.gz
Copyright: GPLv2
Vendor: Joe Yandle <jwy@divisionbyzero.com>
Packager: Mark Horning <rip6@rip6.net>
Buildroot: /var/tmp/gtkmail-%{version}

%description
gtkmail is a GUI email client for X, written in C++ using the gtkmm widget set.
It is fully MIME compliant, and supports standard UNIX mbox style mail files.
As of the 1.0.0 release, gtkmail now supports both mail fetching (IMAP4 and POP3)
and filtering. Also, address books are now supported.  As of 1.0.3, mulitple POP
and IMAP accounts are supported, as well as mail subdirectories.

%prep
%setup -q
./configure --prefix=/usr

%build
make

%install
make prefix=$RPM_BUILD_ROOT/usr install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc AUTHORS COPYING INSTALL README NEWS ChangeLog 
%{_bindir}/gtkmail

%doc AUTHORS COPYING INSTALL README NEWS ChangeLog 


%changelog
* Sat Sep 5 2003  Joe Yandle <jwy@divisionbyzero.com>
- Incremented version to 1.2.0
* Mon Aug 14 2000  Joe Yandle <jwy@divisionbyzero.com>
- Incremented version to 1.0.4
- Updated folder list and MIME to use CTree
- Added pixmaps for folder and MIME trees
- Begin prep for 1.0.4 release
* Thu Jul 6 2000  Joe Yandle <jwy@divisionbyzero.com>
- Added mail-reply-all to pixmaps
* Wed Jun 28 2000 Joe Yandle <jwy@divisionbyzero.com>
- Updated to 1.0.3
- multiple fetch account and email addresses allowed
- subdirectories possible in mail folders
- popup menu for navigation
- gnome style toolbars and icons, with option to remove text

* Sun May 14 2000 Joe Yandle <jwy@divisionbyzero.com>
- Updated to 1.0.2
- Added some stuff that was in CVS but didn't make the 1.0.1 cut

* Sun May 14 2000 Mark Horning <rip6@rip6.net>
- Created spec file
- Configure for RH's structure



