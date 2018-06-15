#!/usr/bin/perl

# EM Feb 17
#
#


use strict;
use warnings;
use Carp;
use Getopt::Std;
#use Data::Dumper;

binmode(STDOUT, ":utf8");

my $progname = "collect-results.pl";

sub usage {
	my $fh = shift;
	$fh = *STDOUT if (!defined $fh);
	print $fh "\n"; 
	print $fh "Usage: $progname [options] <configs dir> <output file>\n";
	print $fh "\n";
	print $fh "   Reads a list of results files (as produced by the official eval script) from\n";
	print $fh "   STDIN (e.g. ls experiments/*/*/cv-eval.out | $progname .....).\n";
 	print $fh "   <configs dir> contains the config files, so that the config ids can be\n";
 	print $fh "   replaced with the values of the parameters.\n";
	print $fh "   the path read from STDIN for every result file should contain the language\n";
	print $fh "   and config id as last two directories, i.e: <.....>/<lang>/<config>/eval.out\n";
# 	print $fh "   works with direct output from official eval script (eval.out) as well as \n";
# 	print $fh "   output from CV script (eval.avg.out)\n";
 	print $fh "\n";
}



sub readConfigFile {
    my $filename=shift;
    open( FILE, '<:encoding(UTF-8)', $filename ) or die "Cannot read config file '$filename'.";
    my %res;
    local $_;
    while (<FILE>) {
        chomp;
        if (m/#/) {
            s/#.*$//;
        }
        s/^\s+//;
        s/\s+$//;
        if ($_) {
            my ($name, $value) = ( $_ =~ m/([^=]+)=(.*)/);
            $res{$name} = $value;
        }
    }
    close(FILE);
    return \%res;
}



sub getConfParams {
    my $config = shift;
    my $names = shift;

    my @res;
    if (scalar(@$names) != scalar(keys %$config)) {
	die "Error: different number of parameters in config";
    }
    foreach my $name (@$names) {
	if (!defined($config->{$name})) {
	    die "Error: parameter '$name' not defined in config";
	}
	push(@res, $config->{$name});
    }
    return join("\t", @res);
}






# PARSING OPTIONS
my %opt;
getopts('h', \%opt ) or  ( print STDERR "Error in options" &&  usage(*STDERR) && exit 1);
usage(*STDOUT) && exit 0 if $opt{h};
print STDERR "2 arguments expected, but ".scalar(@ARGV)." found: ".join(" ; ", @ARGV)  && usage(*STDERR) && exit 1 if (scalar(@ARGV) != 2);

my $configsDir = $ARGV[0]; 
my $outputFile =  $ARGV[1];

my @configNames;

# read configs
opendir(my $dh, $configsDir) || die "Can't open '$configsDir': $!";
my @dots = grep { !/^\./  } readdir($dh);
print "Info: ".scalar(@dots)." configs files found in dir '$configsDir'\n";
my %configs;
foreach my $f (@dots) {
    my ($confId) = ($f =~ m/^(.*)\.conf$/);
    if (!defined($confId)) {
	warn "Warning: ignoring '$f', not a valid config file";
    } else {
#    print STDERR "DEBUG: reading config $f, id = '$confId'\n";
	my $config = readConfigFile("$configsDir/$f");
	if (scalar(@configNames)==0) {
	    @configNames = (sort keys %$config);
	}
	my $confParams = getConfParams($config, \@configNames);
	$configs{$confId} = $confParams;
    }
}
closedir $dh;


# read results files
open(OUT, ">:encoding(utf-8)", $outputFile) or die "Cannot open '$outputFile'";

print OUT "lang\tconfig\t".join("\t", @configNames)."\tlevel\tevalType\tperf\n";
my %langError;

my $nb=0;
while (<STDIN>) {
    chomp;
    my $f= $_;

    # extract language and conf id from the path
    my @parts = split("/", $f);
    pop(@parts);
    my $confId = pop(@parts);
    $confId =~ s/\.output$//;
#    pop(@parts);
    my $lang = pop(@parts);
    my $confStr = $configs{$confId};
    if (!defined($confStr)) {
	die "No config '$confId' found";
    }

    # read content
    open(F, "<:encoding(utf-8)", $f) or die "Cannot open '$f'";
    my $based;
    while (<F>) {
	chomp;
	if (length($_)) { # not empty line
	    if ($_ eq "ERROR") {
		$langError{$lang}++;
	    } elsif (m/^>>/) {
		($based) = (m/>> (.*)-based:/);
	    } elsif (m/\*.*=.*/) {
		my ($evalType, $value) = (m/\* (.) = (\S*)/);
		print OUT "$lang\t$confId\t$confStr\t$based\t$evalType\t$value\n";
	    } else {
		die "Error reading file '$f', line '$_'";
	    }
	}
	
    }
    close(F); 
    $nb++;
}

close(OUT);
print "$nb results files read.\n";
foreach my $l (sort keys %langError) {
    print "  Found $langError{$l} errors for language $l.\n";
}
