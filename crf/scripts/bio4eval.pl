#!/usr/bin/env perl

=head1 NAME

bio4eval.pl -- Transforms a CRF++/Wapiti output file into a ParsemeTSV-formatted file so it can be read by evaluate.py, the official Shared Task evaluation script.

=head1 SYNOPSIS

 bio4eval.pl --input BIOFile --output ParsemeTSVFile

=head1 DESCRIPTION

Transforms a CRF++/Wapiti output file into a ParsemeTSV-formatted file so it can be read by evaluate.py, the official Shared Task evaluation script.

=head1 AUTHOR

Alfredo Maldonado (http://alfredomg.com)

=cut

use strict;
use warnings;
use Getopt::Long;

my $input = "";
my $output = "";
GetOptions(
    'input:s' => \$input,
    'output:s' => \$output
		);

open(my $input_fo,   "<:utf8", $input)   or die "Cannot open '$input': $!\n";
open(my $output_fo, ">:utf8", $output) or die "Cannot create '$output': $!\n";
my $n = 0; my $topNformat = 0; my $sentID = -1;
while (my $line = <$input_fo>)
{
    if ($line =~ /^\s*$/ && (!$topNformat || $sentID == 0))
    {
	print $output_fo $line;
	$n = 0;
    }
    elsif ($line =~ /^#/) # comment or top N line from crf_test
    {
	if ($line =~ /^# (\d+)/) # top N line from crf_test
	{
	    $sentID = int($1);
	    $topNformat = 1;
	}
    }
    elsif (!$topNformat || $sentID == 0)
    {
	chomp $line;
	my @fields = split(/\s+/, $line);
	my $tokenID = $fields[0];
	my $token = $fields[1];
	my $label = $fields[$#fields];
	if ($label !~ /^[_\d]/)
	{
	    if ($label eq "B")
	    {
		$n++;
		$label = $n;
	    }
	    elsif ($label eq "I")
	    {
		$label = $n;
	    }
	    else # anything else
	    {
		$label = 1;
	    }
	}
	print $output_fo "$tokenID\t$token\t_\t$label\n";
    }
}
close($input_fo);
close($output_fo);

