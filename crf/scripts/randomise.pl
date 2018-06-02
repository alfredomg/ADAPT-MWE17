#!/usr/bin/env perl

=head1 NAME

randomise.pl -- Randomises a ParsemeTSV and optionally a corresponding CONLLU file

=head1 SYNOPSIS

 randomise.pl --ptsv InputParsemeTSVFile [--conll InputConlluFile] --outpref OutputPrefix [--seed RandomSeed]

=head1 DESCRIPTION

Randomises a ParsemeTSV and optionally a corresponding CONLLU file

--ptsv and --conll input files in ParsemeTSV and CONLLU formats (CONLLU file optional)

--outpref Output file(s) prefix. Output files will be OutputPrefix.parsemetsv and OutputPrefix.conllu, respectively

--seed A numeric seed. Optional. Use only for debugging. If not specified, internal randomisation will be performed.

=head1 AUTHOR

Alfredo Maldonado (http://alfredomg.com)

=cut

use strict;
use warnings;
use Getopt::Long;
use FindBin; # locate this script
use lib "$FindBin::RealBin/."; # include current dir for modules
use Common;

my $ptsv = "";
my $conll = "";
my $outpref = "./output";
my $seed = "";
GetOptions(
    'ptsv:s' => \$ptsv,
    'conll:s' => \$conll,
    'outpref:s' => \$outpref,
    'seed:s' => \$seed
);

my $nSeqs = countSequences($ptsv);

my @sequences = ();
$#sequences = $nSeqs - 1;

# Read sentences into array

open(my $pfo,   "<:utf8", $ptsv) or die "Cannot open '$ptsv': $!\n";
(open(my $cfo,   "<:utf8", $conll) or die "Cannot open '$conll': $!\n") if ($conll);
my $lix = 0;
my $pbuffer = "";
my $cbuffer = "";
my $last = "";
while (my $pline = <$pfo>)
{
    my $cline = <$cfo> if ($conll);
    
    $pbuffer .= $pline;
    $cbuffer .= $cline if ($conll);
    
    if ($last ne $pline && $pline =~ /^\s*$/)
    {
	$sequences[$lix] = [ $pbuffer, $cbuffer ];
	$lix++;
	$pbuffer = "";
	$cbuffer = "";
    }
    
    $last = $pline;
}
if (length($pbuffer) > 0) # in case the last sentence didn't end with a blank line.
{
    $sequences[$lix] = [ $pbuffer, $cbuffer ];
}
close($pfo);
close($cfo) if ($conll);

# Randomly select sentences from array and write them to output files

if ($seed)
{
    srand($seed);
}
else
{
    srand (time ^ $$ ^ unpack "%L*", `ps axww | gzip`);
}

my $outp = "$outpref.parsemetsv";
my $outc = "$outpref.conllu";
open(my $outpfo,   ">:utf8", $outp) or die "Cannot create '$outp': $!\n";
(open(my $outcfo,   ">:utf8", $outc) or die "Cannot create '$outc': $!\n") if ($conll);
my $pendSeqs = $nSeqs;
while ($pendSeqs > 1)
{
    my $drawnSeqIx = int(rand($nSeqs));
    my $drawnSeq = $sequences[$drawnSeqIx];
    if (defined $drawnSeq)
    {
	my $pSeq = $$drawnSeq[0];
	my $cSeq = $$drawnSeq[1];
	print $outpfo $pSeq;
	print $outcfo $cSeq if ($conll);
	undef $sequences[$drawnSeqIx];
	$pendSeqs--;
    }
}
close($outpfo);
close($outcfo) if ($conll);



