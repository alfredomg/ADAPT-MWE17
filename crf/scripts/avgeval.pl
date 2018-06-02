#!/usr/bin/env perl

=head1 NAME

avgeval.pl -- Averages evaluate.py scores

=head1 SYNOPSIS

 avgeval.pl EVALFILES

=head1 DESCRIPTION

Averages evaluate.py scores

Example:

 avgeval.pl eval.*

=head1 AUTHOR

Alfredo Maldonado (http://alfredomg.com)

=cut

use strict;
use warnings;
use FindBin; # locate this script
use lib "$FindBin::RealBin/."; # include current dir for modules
use CommandLineFiles;

my @evalFiles = GetFilesFromCommandLine();

my $amwe_p = 0; my $amwe_r = 0; my $amwe_f = 0; my $atok_p = 0; my $atok_r = 0; my $atok_f = 0;
my $iter = 0;
foreach my $evalFile (@evalFiles)
{
    my ($mwe_p, $mwe_r, $mwe_f, $tok_p, $tok_r, $tok_f) = readEvalFile($evalFile);
    $amwe_p += $mwe_p;
    $amwe_r += $mwe_r;
    $amwe_f += $mwe_f;
    $atok_p += $tok_p;
    $atok_r += $tok_r;
    $atok_f += $tok_f;
    $iter++;
}
$amwe_p /= $iter;
$amwe_r /= $iter;
$amwe_f /= $iter;
$atok_p /= $iter;
$atok_r /= $iter;
$atok_f /= $iter;

print ">> MWE-based:\n";
print "  * P = $amwe_p\n";
print "  * R = $amwe_r\n";
print "  * F = $amwe_f\n";
print "\n";
print ">> Token-based:\n";
print "  * P = $atok_p\n";
print "  * R = $atok_r\n";
print "  * F = $atok_f\n";

sub readEvalFile
{
    my $evalFile = shift;
    
    my @values = (0) x 6;
    open(my $fo,   "<:utf8", $evalFile) or die "Cannot open '$evalFile': $!\n";
    my $tokenbased;
    while (my $line = <$fo>)
    {
	chomp $line;
	if ($line !~ /^>>/)
	{
	    if ($line =~ /(\w) = (\d\.\d+)/)
	    {
		my $valtype = $1;
		my $value = $2;
		my $valix = $valtype eq "P" ? 0 : $valtype eq "R" ? 1 : 2;
		$valix += 3 if ($tokenbased);
		$values[$valix] = $value;
	    }
	}
	else
	{
	    $tokenbased = ($line =~ /Token-based/);
	}
    }
    close($fo);
    
    return @values;
}


