# Note that this is NOT a relocatable package
%define ver      0.6
%define rel      1
%define prefix   /usr

Summary: Driver and diagnostic utility for Usermode SoundModem
Name: soundmodem
Version: %ver
Release: %rel
Copyright: GPL
Group: Networking/Hamradio
Source: soundmodem-%{ver}.tar.gz
BuildRoot: /tmp/soundmodem-root
Packager: Thomas Sailer <sailer@ife.ee.ethz.ch>
# Requires: /sbin/ifconfig /sbin/route /sbin/arp

%description
This package contains the driver and the diagnostic utility for
userspace SoundModem. It allows you to use soundcards supported
by OSS/Free as Amateur Packet Radio modems.

%prep
%setup

%build
%configure --enable-mmx
make

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall
install -g root -o root -m 0755 -d $RPM_BUILD_ROOT/etc/rc.d/init.d
install -g root -o root -m 0755 soundmodem.initscript $RPM_BUILD_ROOT/etc/rc.d/init.d/soundmodem
install -g root -o root -m 0755 -d $RPM_BUILD_ROOT/etc/ax25
touch $RPM_BUILD_ROOT/etc/ax25/soundmodem.conf
chmod 0600 $RPM_BUILD_ROOT/etc/ax25/soundmodem.conf
chown 0.0 $RPM_BUILD_ROOT/etc/ax25/soundmodem.conf

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)

%doc AUTHORS COPYING ChangeLog INSTALL NEWS README newqpsk/README.newqpsk
%{prefix}/sbin/*
%{prefix}/bin/*
%{prefix}/share/man/*/*
%config /etc/rc.d/init.d/soundmodem
%config /etc/ax25/soundmodem.conf
