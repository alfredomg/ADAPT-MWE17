#!/usr/bin/env perl

=head1 NAME

parseme2bio.pl -- Transforms a parsemetsv file into a BIO file ready to use with Watipi/CRF++

=head1 SYNOPSIS

 parseme2bio.pl --ptsv ParsemeTSVFile [--conll ConllFile | --tt TreeTaggerFile] [ --multilabel ] [ --bio ] [ --oneclass CLASS ] --output BIOFile

=head1 DESCRIPTION

Transforms a parsemetsv file into a BIO file ready to use with Watipi/CRF++

If neither --conll nor --tt file is set, dummy _ values are output for pos, and surface is copied for lemma

If --multilabel is specified, labels are the classes used in ParsemeTSVFile (LVC, ID, IReflV, VPC, OTH, etc.) If not specified, a generic label is used instead.

If --bio is specified, then the generic label used is not numeric and instead B and I are used.

If --oneclass CLASS is specified, only labels for CLASS are output. All other classes are output as _.

=head1 AUTHOR

Alfredo Maldonado (http://alfredomg.com)

=cut

use strict;
use warnings;
use Getopt::Long;

my $ptsv = "";
my $conll = "";
my $tt = "";
my $multilabel = "";
my $bio = "";
my $oneclass = "";
my $output = "";
GetOptions(
    'ptsv:s' => \$ptsv,
    'conll:s' => \$conll,
    'tt:s' => \$tt,
    'multilabel' => \$multilabel,
    'bios' => \$bio,
    'oneclass:s' => \$oneclass,
    'output:s' => \$output
		);

die "Parseme TSV file (--ptsv) does not exist: $ptsv\n" if (!-f $ptsv);
if ($tt && $conll)
{
    die "You cannot specify both a CONLLU file (--conll) and a TreeTagger file (--tt) at the same time. Please specify only one!\n";
}
if ($conll)
{
    if (!-f $conll)
    {
        $conll = "";
        print "CONLLU file (--conll) does not exist: '$conll' -- Dummy values will be used\n";
    }
}
if ($tt)
{
    if (!-f $tt)
    {
        $tt = "";
        print "TreeTagger file (--tt) does not exist: '$tt' -- Dummy values will be used\n";
    }
}

my $parsed = $conll ? $conll : $tt ? $tt : "";

open(my $ptsv_fo,   "<:utf8", $ptsv) or die "Cannot open '$ptsv': $!\n";
(open(my $conll_fo,   "<:utf8", $parsed) or die "Cannot open '$parsed': $!\n") if ($parsed);
open(my $output_fo, ">:utf8", $output) or die "Cannot create '$output': $!\n";
my $linenum = 1; my $cline; my %id2cfields = (); my @sentenceBuffer = ();
while (my $line = <$ptsv_fo>)
{
    $cline = <$conll_fo> if ($conll);
    chomp $cline if ($conll);
    
    if ($line =~ /^\s*$/)
    {
	printSentence($output_fo, \@sentenceBuffer, \%id2cfields);
	print $output_fo $line;
	%id2cfields = () if ($conll);
	@sentenceBuffer = ();
    }
    elsif ($line =~ /^#/) # comment
    {
    }
    else
    {
	chomp $line;
	$cline = <$conll_fo> if ($tt);
	chomp $cline if ($tt);
	my @fields = split(/\t/, $line);
	handleSpacesAndMissingValues(\@fields);
	my $numfields = scalar(@fields);
	if ($numfields != 4)
	{
	    print STDERR "WARNING: Line $linenum in '$ptsv' has $numfields and not 4. Considering label field to be the last one, i.e. the $numfields th column\n";
	}
	my @cfields = split(/\t/, $cline) if ($parsed);
	handleSpacesAndMissingValues(\@cfields) if ($parsed);
	my $tokenID = $fields[0];
	my $surface = $fields[1];
	my $label = $fields[$numfields - 1];
	my $lemma = $conll ? $cfields[2] : $tt ? $cfields[2] : $fields[1];
	my $pos   = $conll ? $cfields[3] : $tt ? $cfields[1] : "_";
	my $headID = "_";
	my $headdeprel = "_";
	if ($conll) # setting $headlemma and $headdeprel
	{
	    $id2cfields{$tokenID} = \@cfields;
	    $headID = $cfields[6];
	    $headdeprel = $cfields[7];
	}
	my $outlabel = handlelabel($label);
	push @sentenceBuffer, [$tokenID, $surface, $pos, $lemma, $headID, $headdeprel, $outlabel];
    }
    $linenum++;
}
printSentence($output_fo, \@sentenceBuffer, \%id2cfields) if (scalar(@sentenceBuffer) > 0);
close($ptsv_fo);
close($conll_fo) if ($parsed);
close($output_fo);

sub printSentence
{
    my $output_fo = shift; 
    my $sentenceBuffer = shift;
    my $id2cfields = shift;
    
    foreach my $tokenEntry (@$sentenceBuffer)
    {
    if ($conll)
    {
        my $tokenID = $$tokenEntry[0];
        my $headID = $$tokenEntry[4];
        my $headLemma = "_";
        my $headpos = "_";
        if ($headID ne "_" && $tokenID !~ /\-/ && $headID ne "0")
        {
            my $headcfields = $$id2cfields{$headID};
            $headLemma = $$headcfields[2];
            $headpos = $$headcfields[3];
        }
        elsif ($headID eq "0") # the head is the root
        {
            $headLemma = "<[R00T]>";
            $headpos = "<[R00T]>";
        }
        if (!defined $headLemma) # this happens with 1 line in Turkish data. seems to be an error, just assume _
        {
            $headLemma = "_";
            $headpos = "_";
            #print "$linenum\n";
            #print join("\t", @$tokenEntry) . "\n";
            #exit 1;
        }
        
        $$tokenEntry[4] = "$headLemma\t$headpos";
    }
    else
    {
        $$tokenEntry[4] = "_\t_";  # to preserve the same number of columns
    }

    print $output_fo join("\t", @$tokenEntry) . "\n";
    }
}

sub handleSpacesAndMissingValues
{
    my $fields = shift;
    
    for (my $i = 0; $i < scalar(@$fields); $i++)
    {
	$$fields[$i] = "_" if ($$fields[$i] =~ /^\s*$/); # change empty field to underscore
	$$fields[$i] =~ s/\s/_/g; # change spaces to underscores
    }
}

my $curLabel = "";
sub handlelabel # NOTICE: Does not handle embedded labels
{
    my $label = shift;
       
    if ($label =~ /^\d+/)
    {
	my $initialToken = 0; 
	if ($label =~ /^\d+:(\w+)/)
	{
	    $curLabel = $1;
	    $initialToken = 1;
	}
	if ($oneclass && $oneclass ne $curLabel)
	{
	    return "_";
	}
	my $outlabel = "1";
	if ($bio)
	{
	    $outlabel = $initialToken ? "B" : "I";
	}
	if ($multilabel)
	{
	    $outlabel .= ":$curLabel";
	}
	return $outlabel;
    }
    
    return "_";
}

