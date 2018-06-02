#!/usr/bin/env perl

=head1 NAME

split.pl -- Splits a bio file (produced by parseme2bio.pl) into several subfiles

=head1 SYNOPSIS

 split.pl --input BIOFile [--split SPLITDESC]

=head1 DESCRIPTION

Splits a bio file (produced by parseme2bio.pl) into several subfiles.

--split : Proportions of splits. E.g. "0.5,0.25,0.25" (default value) splits in three ways with described proportions.

Currently only sequential splits are done (does not randomise).

Output files are called BIOFile.1, BIOFile.2, etc. each corresponding to a proportion in the order described in --split

=head1 AUTHOR

Alfredo Maldonado (http://alfredomg.com)

=cut

use strict;
use warnings;
use Getopt::Long;
use FindBin; # locate this script
use lib "$FindBin::RealBin/."; # include current dir for modules
use Common;

my $input = "";
my $split = "0.5,0.25,0.25";
GetOptions(
    'input:s' => \$input,
    'split:s' => \$split
);

my @props = split(/\s*,\s*/, $split);
@props = checkProps(\@props);
my $nSeqs = countSequences($input);
my @nSplits = numPerSplits($nSeqs, \@props);

divide($input, \@nSplits);

sub divide
{
    my $file = shift;
    my $nSplits = shift;
    
    open(my $fo,   "<:utf8", $file) or die "Cannot open '$file': $!\n";
    my $nSeqs = 0;
    my $last;
    my $curSplit = 0;
    open(my $outfo,   ">:utf8", "$file.$curSplit") or die "Cannot create '$file.$curSplit': $!\n";
    while (my $line = <$fo>)
    {
	print $outfo $line;
	if (!defined $last && $line !~ /^\s*$/)
	{
	    $nSeqs++;
	}
	elsif ($last ne $line && $line =~ /^\s*$/)
	{
	    $nSeqs++;
	}
	
	if ($nSeqs == $$nSplits[$curSplit] && $curSplit < scalar(@$nSplits) - 1)
	{
	    close($outfo);
	    $curSplit++;
	    open($outfo,   ">:utf8", "$file.$curSplit") or die "Cannot create '$file.$curSplit': $!\n";
	    $nSeqs = 0;
	}
	
	$last = $line;
    }
    close($fo);
    close($outfo);
}

sub numPerSplits
{
    my $nSeqs = shift;
    my $props = shift;
    
    my @nSplits = ();
    foreach my $p (@$props)
    {
	my $num = int($p * $nSeqs);
	push @nSplits, $num;
    }
    return @nSplits;
}

sub checkProps
{
    my $ps = shift;
    
    my @nums = ();
    
    my $sum = 0;
    foreach my $p (@$ps)
    {
	my $num = 0.0 + $p;
	$sum += $num;
	push @nums, $num;
    }
    
    if ($sum != 1)
    {
	die "Proportions do not sum to 1";
    }
    
    return @nums;
}


