#!/usr/bin/env perl

=head1 NAME

headAvgeval2Tab.pl -- Transforms average evals produced by  head */*/eval.avg to a tabular format 

=head1 SYNOPSIS

 headAvgeval2Tab.pl --head FileProducedByHead [--score SCORETYPE] [--count]

=head1 DESCRIPTION

Transforms average evals produced by  head */*/eval.avg to a tabular format

Example:

 headAvgeval2Tab.pl --head FileProducedByHead --score F
 
--score can be either P, R or F. (Default: F)

If --count is specified, it also reports the amount of positive examples in the evaluated set, as indicated in the denominator of the Recall score. Can only be obtained from direct evaluate.py scores and not from averages. 

=head1 AUTHOR

Alfredo Maldonado (http://alfredomg.com)

=cut

use strict;
use warnings;
use Getopt::Long;

my $head = "";
my $score = "F";
my $count = "";
GetOptions
(
	"head:s" => \$head,
	"score:s" => \$score,
	"count" => \$count
);

my %scores = ();

open(my $fo, "<:utf8", $head) or die "Cannot open '$head': $!\n";
my $template_score;
my $scope; my $rcount;
while (my $line = <$fo>)
{
    chomp $line;
    if ($line =~ /==> (\w+)\/([\w\.]+)/)
    {
	my $language = $1;
	my $template = $2;
	
	my $language_score = $scores{$language};
	if (!defined $language_score)
	{
	    $language_score = {};
	    $scores{$language} = $language_score;
	}
	$template_score = $$language_score{$template};
	if (!defined $template_score)
	{
	    $template_score = {};
	    $$language_score{$template} = $template_score;
	}
    }
    elsif ($line =~ />> ([\w\-]+):/)
    {
	$scope = $1;
    }
    elsif ($count && $line =~ /\* R = \d\.\d+ \(\d+ \/ (\d+)\)/)
    {
	$rcount = $1;
    }
    elsif ($line =~ /\* (\w) = (\d\.\d+)/)
    {
	my $stype = $1;
	my $value = $2;
	if ($stype eq $score)
	{
	    $$template_score{$scope} = $count ? { 'value' => $value, 'count' => $rcount } : $value;
	}
    }
}
close($fo);

print "Score type:\t$score\n\n";
print "Lang\tScope\t";
my %templates = ();
foreach my $language (keys %scores)
{
    foreach my $template (keys %{$scores{$language}})
    {
	$templates{$template} = 1;
    }
}
my @ordTemplates = sort keys %templates;
my $strTemplates = "";
foreach my $template (@ordTemplates)
{
    $strTemplates .= "$template\t" . ($count ? "\t" : "");
}
chop $strTemplates;
print "$strTemplates\n";
if ($count)
{
    $strTemplates = "\t\t";
    foreach my $template (@ordTemplates)
    {
	$strTemplates .= "count\t$score\t";
    }
    chop $strTemplates;
    print "$strTemplates\n";
}
foreach my $language (sort keys %scores)
{
    my $language_score = $scores{$language};
    my %linePerScope = ();
    foreach my $template (@ordTemplates)
    {
	$template_score = $$language_score{$template};
	if (defined $template_score && scalar(keys %$template_score) > 0)
	{
	    foreach $scope (keys %$template_score)
	    {
		my $value = $$template_score{$scope};
		$linePerScope{$scope} .= ($value ? ($count ? $$value{'count'} . "\t" . $$value{'value'} : $value) : ($count ? "N/A\tN/A" : "N/A")) . "\t";
	    }
	}
	else
	{
	    foreach $scope (keys %linePerScope)
	    {
		$linePerScope{$scope} .= "N/A\t";
	    }
	}
    }
    my $first = 1;
    foreach $scope (sort keys %linePerScope)
    {
	my $line_scope = $linePerScope{$scope};
	chop $line_scope;
	print "$language" if ($first);
	print "\t$scope\t$line_scope\n";
	$first = 0;
    }
}

