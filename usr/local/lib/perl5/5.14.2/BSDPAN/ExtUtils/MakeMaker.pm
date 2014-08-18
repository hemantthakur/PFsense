# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 42)
# <tobez@FreeBSD.org> wrote this file.  As long as you retain this notice you
# can do whatever you want with this stuff. If we meet some day, and you think
# this stuff is worth it, you can buy me a beer in return.   Anton Berezin
# ----------------------------------------------------------------------------
#
# $Id: MakeMaker.pm 44 2011-11-07 08:34:26Z sskvortsov $
#
package BSDPAN::ExtUtils::MakeMaker;
#
# The pod documentation for this module is at the end of this file.
#
use strict;
use Carp;
use BSDPAN;
use BSDPAN::Override;
use File::Basename;

our $VERSION = $ExtUtils::MakeMaker::VERSION;

sub WriteMakefile
{
	my $orig = shift;
	my %p = @_;

	if ($p{NAME} && -d $ENV{P5PORTER} && -r _ && -w _) {

		require YAML;
		my $modname = $p{NAME};
		my $portname = $modname;
		$portname =~ s/::/-/g;
		my $cfg = "$ENV{P5PORTER}/$portname.cfg";
		my $C = {};
		if (-f $cfg && -r _) {
			if (open my $fh, '<', $cfg) {
				local $/;
				my $yaml = <$fh>;
				$C = YAML::Load($yaml);
				close $fh;
			}
		}
		$C->{portname} = $portname;
		$C->{modname} = $modname;
		$C->{dependencies} = [];

		if ($p{VERSION}) {
			$C->{version} = $p{VERSION};
		} elsif ($p{VERSION_FROM} && -f $p{VERSION_FROM} && -r _) {
			$C->{version} = ExtUtils::MM_Unix->parse_version($p{VERSION_FROM});
		}
		$C->{portversion} = $C->{version};
		if ($modname =~ /^([^:]+)(::)?/) {
			$C->{subdir} = $1;
		}

		require POSIX;
		POSIX::setlocale(&POSIX::LC_TIME, "C");
		$C->{today} = POSIX::strftime("%d %B %Y", localtime);

		if ($p{PREREQ_PM}) {
			my $PORTSDIR = $ENV{'PORTSDIR'} // '/usr/ports';
			for my $req (keys %{$p{PREREQ_PM}}) {
				my $mname = "$req.pm";
				$mname =~ s|::|/|g;
				my @r = `/usr/bin/grep '%/$mname\$' $PORTSDIR/lang/perl5.14/pkg-plist`;
				if (@r) {
					print "BSDPAN: Found $req in base perl\n";
				} else {
					my $cand = $req;
					my $req_ver = ${$p{PREREQ_PM}}{$req};
					$cand =~ s/::.*$//;
					my @cand = `ls $PORTSDIR/*/p5-$cand*/pkg-plist`;
					my @ports;
					my @files;
					for my $cand (@cand) {
						chomp $cand;
						my @r = `/usr/bin/grep '%/$mname\$' $cand`;
						for (@r) {
							chomp;
							push @ports, $cand;
							push @files, $_;
						}
					}
					for (@files) {
						s/%%([^%]+)%%/\${$1}/g;
					}
					for (@ports) {
						s|^/usr/ports/|\${PORTSDIR}/|;
						s|/pkg-plist$||;
					}
					if (@ports) {
						print "BSDPAN: found $req in @ports\n";
						my $PKG_DBDIR = $ENV{'PKG_DBDIR'} // '/var/db/pkg';
						for my $port (@ports) {
							$_ = $port;
							s|.*/||;
							my @r = `ls -d $PKG_DBDIR/$_-* 2>/dev/null`;
							my $p5 = $_;
							if (@r && grep { /\Q$p5\E-\d/ } @r) {
								print "BSDPAN: $p5 is already installed\n";
							} else {
								print "BSDPAN: $p5 is NOT installed, so go install it!\n";
							}
						}
					} else {
						print "BSDPAN: no port found for $req, so do it by hand\n";
					}
					while (@ports) {
						my $p = shift @ports;
						my $f = shift @files;
						my $pn = basename $p;
						push @{$C->{dependencies}}, "$pn>=$req_ver:$p";
					}
					$C->{freebsd_dependencies} = join " \\\n\t\t", sort @{$C->{dependencies}};
				}
			}
		}
		$C->{has_dependencies} = @{$C->{dependencies}} > 0;

		if (open my $fh, '>', $cfg) {
			print $fh YAML::Dump($C);
			close $fh; 
		}

	}

	my @r = &$orig;
	return @r;
}

BEGIN {
	if ($ENV{P5PORTER}) {
		override 'WriteMakefile', \&WriteMakefile;
	}
}

1;
=head1 NAME

BSDPAN::ExtUtils::MakeMaker - Override ExtUtils::MakeMaker functionality

=head1 SYNOPSIS

   None

=head1 DESCRIPTION

BSDPAN::ExtUtils::MakeMaker overrides one sub from the standard perl
module ExtUtils::MakeMaker.

If P5PORTER environment variable is not defined, the overridden
WriteMakefile simply calls the original WriteMakefile.  If, however,
this environment variable is defined, this module assumes that the
automated perl port creation is requested.  At present, this mode of
operation is not documented.

=head1 AUTHOR

Anton Berezin, tobez@FreeBSD.org

=head1 SEE ALSO

C<perl(1)>, L<BSDPAN>, L<BSDPAN::Override>, C<ports(7)>.

=cut
