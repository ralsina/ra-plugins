# Initial spec file created by autospec ver. 0.8 with rpm 3 compatibility
Summary: plugins-ra is a collection of qmail-spp plugins
Name: plugins-ra
Version: 0.2.2
Release: 1ra
License: GPL
Group: Applications/Mail
Source: ra-plugins-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-root
# Following are optional fields
#URL: http://www.example.net/plugins/
Requires: qmail-common
#Obsoletes: 
BuildRequires: vpopmail, mysql-devel, libspf2-devel, relayd

%description
  This package provides several plugins meant to be used with a qmail patched using qmail-spp
  (http://qmail-spp.sf.net).

  They cover diverse areas, and are meant to be written according to the following principles:

  * Good performance

  They are written whenever possible in C, and try to be short and direct.

  * Good logging

  They should inform the user of what they do. No email should ever be rejected without logging
  **something**. Also, all plugins logs include the PID of the qmail-smtpd process, so you can
  easily group all events of a connection.

  * Security

  Since writing in C exposes oneself to the dangers of memory corruption, I have tried to use,
  whenever it makes sense, the Better String Library, to avoid problems. While this makes the
  plugins larger, it also makes me feel more comfortable.

%prep
%setup -n ra-plugins-%{version}

%build
make

%install
make install DESTDIR=$RPM_BUILD_ROOT

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf "$RPM_BUILD_ROOT"

%files
%defattr(-,root,root)
%dir /var/qmail/plugins
/var/qmail/plugins/authchecks
/var/qmail/plugins/rcptchecks
/var/qmail/plugins/rblchecks
/var/qmail/plugins/mfdnschecks
/var/qmail/plugins/ipthrottle
/var/qmail/plugins/hardcoderbl
/var/qmail/plugins/authlogger
/var/qmail/plugins/spfchecks
/var/qmail/plugins/msa
/var/qmail/plugins/localmail
/var/qmail/plugins/tarpit
%dir %{_mandir}/man8
%doc %{_mandir}/man8/plugins-ra.8.gz
%doc %{_mandir}/man8/authchecks.8.gz
%doc %{_mandir}/man8/rcptchecks.8.gz
%doc %{_mandir}/man8/rblchecks.8.gz
%doc %{_mandir}/man8/mfdnschecks.8.gz
%doc %{_mandir}/man8/ipthrottle.8.gz
%doc %{_mandir}/man8/authlogger.8.gz
%doc %{_mandir}/man8/hardcoderbl.8.gz
%doc %{_mandir}/man8/spfchecks.8.gz
%doc %{_mandir}/man8/msa.8.gz
%doc %{_mandir}/man8/localmail.8.gz
%doc %{_mandir}/man8/tarpit.8.gz
%doc COPYING
%doc README
%doc authchecks.man.txt
%doc authlogger.man.txt
%doc bstrlib.txt
%doc hardcoderbl.man.txt
%doc ipthrottle.man.txt
%doc localmail.man.txt
%doc man-template.txt
%doc mfdnschecks.man.txt
%doc msa.man.txt
%doc plugins-ra.man.txt
%doc rblchecks.man.txt
%doc rcptchecks.man.txt
%doc spfchecks.man.txt
%doc tarpit.man.txt

%changelog
* Mon Mar 27 2006 Roberto Alsina <ralsina@kde.org>
- Initial spec file created by autospec ver. 0.8 with rpm 3 compatibility
