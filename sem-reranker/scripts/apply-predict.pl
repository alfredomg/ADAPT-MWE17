#!/usr/bin/perl

# EM January 17
#
#


use strict;
use warnings;
use Carp;
use Getopt::Std;
#use Data::Dumper;

binmode(STDOUT, ":utf8");

my $progname = "apply-predict.pl";

my $tokenOrLemmaCol = 3; # 1 for token, 3 for lemma
my $goldCol = 7;
my $predCol = 8;

my $nanStr = "NA";

sub usage {
	my $fh = shift;
	$fh = *STDOUT if (!defined $fh);
	print $fh "\n"; 
	print $fh "Usage: $progname [options] <weka predict tsv file> <input with multiple answers> <output>\n";
	print $fh "\n";
	print $fh "  Applies the predictions from the semantic reranker, i.e. converts from <weka predict tsv file>,\n";
	print $fh "  which contains one instance by line (an instance is one of the 10 alternative sequences for\n";
	print $fh "  one sentence), to the conll BIO format, as given is <input with multiple answers> which is \n";
	print $fh "  the CRF output with 10 sequences by sentence.\n";
 	print $fh "\n";
 	print $fh "  Options:\n";
 	print $fh "    -a <analysis output file>: requires that the gold labels are present in\n";
	print $fh "        <input with multiple answers>; these are used to output data for error analysis.\n";
 	print $fh "    -c <coverage feature col no>: useful only with '-a'. Adds 'coverage' columns based on\n";
 	print $fh "       the feature found in the given column number from the weka file.\n";
	print $fh "    -g <BIO gold file> only useful with -a, in case the gold labels are not given in the\n";
	print $fh "       gold column in <input with multiple answers>. This file contains every sentence only\n";
	print $fh "       once.\n";
 	print $fh "\n";
}


#
# Given sentence labels as an array, e.g. _ _ _ B I _ I _ _ B I _ _, returns: $res->[exprNo]->[wordExprNo] = indexInSentence,
# e.g. [ [3,4,6], [9,10] ]
#
#
#
sub exprIndexes {
    my ($labels) = @_;
#    print STDERR "DEBUG exprIndexes: labels  = ".join(";", @$labels)."\n";
    my @current =();
    my @res;
    for (my $i=0; $i<scalar(@$labels); $i++) {
	if (($labels->[$i] eq "B") || ($labels->[$i] eq "I") || ($labels->[$i] eq "_")) {
	    if (($labels->[$i] eq "B") || ($labels->[$i] eq "I")) { # expr label
		if (($labels->[$i] eq "B") && (scalar(@current)>0)) { # start of new expr with previously existing one
		    my @copyCurrent = @current;
		    push(@res, \@copyCurrent);
		    @current = ();
		}
		push(@current, $i);
	    }
	} else {
	    die "Error in exprIndexes: invalid character label '".$labels->[$i]."'";
	}
	
    }
    if (scalar(@current)>0) { # last expr
	push(@res, \@current);
    }
    return \@res;
}



sub identicalExprs {
    my ($exprs1, $exprs2) = @_;

    if (scalar(@$exprs1) != scalar(@$exprs2)) {
	return 0;
    } else {
	for (my $i=0; $i<scalar(@$exprs1); $i++) {
	    my $expr1 = $exprs1->[$i];
	    my $expr2 = $exprs2->[$i];
	    if (scalar(@$expr1) != scalar(@$expr2)) {
		return 0;
	    } else {
		for (my $j=0; $j<scalar(@$expr1); $j++) {
		    if ($expr1->[$j] != $expr2->[$j]) {
			return 0;
		    }
		}
	    }
	}
	return 1;
    }
}



#
# from expressions indexes (see exprIndexes output format) and the sequence of words in the sentence, returns
# a string representing all the expressions as words.
#
# if $sepAfterNbExprs is defined, add <nb expression><separator><proportion continuous><separator>  before the expression as string itself
#
sub exprsAsString {
    my ($exprs, $words, $sepAfterNbExprs) = @_;

    my @parts;
    my $nbConti=0;
    foreach my $expr (@$exprs) {
#	print STDERR "debug expr\n";
	my @exprStr;
	my $contiThis = 1;
	for (my $wordNo=0; $wordNo<scalar(@$expr);  $wordNo++) {
	    my $index = $expr->[$wordNo];
#	    print STDERR "debug expr wordNo=$wordNo; index=$index; word=".$words->[$index]."\n";
	    if ($wordNo>0) { # not for first word
		if ($expr->[$wordNo-1] < $index-1) {
		    $contiThis=0;
		}
	    }
	    push(@exprStr, $words->[$index]);
	}
	push(@parts, join(" ",@exprStr));
	$nbConti += $contiThis; # adds 1 if conti=true, 0 if conti=false
    }
    my $exprsStr = "[".join("|", @parts)."]";
#    print "debug: exprsStr = '$exprsStr'\n";
    if (defined($sepAfterNbExprs)) {
	my $propConti = (scalar(@parts)==0) ? $nanStr : $nbConti / scalar(@parts);
	$exprsStr =  scalar(@parts).$sepAfterNbExprs.$propConti.$sepAfterNbExprs.$exprsStr;
    }
    return $exprsStr;
}


sub readExternalGoldFile {
    my ($f)=@_;
    open(F, "<:encoding(utf-8)", $f) or die "Cannot open '$f'";

    my $sentNo=0;
    my @columnsData; # data[sentNo]->[lineNo]->[colNo] = content of column
    while (<F>) {
	chomp;
	if (m/^#/) {
	    die "Error reading external gold file: no comments expected in this file";
	}
	if (m/^\s*$/) { # empty line=new sentence
	    $sentNo++;
	} else {
	    my $line = $_;
	    # store columns data (for analysis later)
	    my @cols = split("\t", $line);
# update: this was not good, since without spaces it's impossible to know if 1-2 followed by 1 is new sentence or not (too complicated at least)
#	    if (($cols[0] eq "1") || ($cols[0] =~ m/^1-/)) { # id word = 1 -> new sentence. update: sometimes it's 1-2 if contraction!
#		$sentNo++;
#	    }
#	    print STDERR "DEBUG READGOLD: sentNo=$sentNo, cols=".join(";",@cols)." \n";
	    # actually we only need the gold col but we store everything, who cares
	    push(@{$columnsData[$sentNo]}, \@cols);
	}
    }
    close(F);
    return \@columnsData;
}




# PARSING OPTIONS
my %opt;
getopts('ha:c:g:', \%opt ) or  ( print STDERR "Error in options" &&  usage(*STDERR) && exit 1);
usage(*STDOUT) && exit 0 if $opt{h};
print STDERR "3 arguments expected, but ".scalar(@ARGV)." found: ".join(" ; ", @ARGV)  && usage(*STDERR) && exit 1 if (scalar(@ARGV) != 3);


my $wekaFile =  $ARGV[0];
my $inputFile =  $ARGV[1];
my $outputFile =  $ARGV[2];

my $analysisOutput = $opt{a};
my $goldFileAnalysis = $opt{g};
my $coverageCol = $opt{c};

open(OUT, ">:encoding(utf-8)", $outputFile) or die "Cannot open '$outputFile'";

open(F, "<:encoding(utf-8)", $wekaFile) or die "Cannot open '$wekaFile'";

my @sentsScores; # $sentsScores[$sentNo]->{$altNo} = $score;
my @coverage; # for reference corpus coverage feature, see option -c;  $coverage[$sentNo]->{$altNo}= feature value
while (<F>) {
    chomp;
    my @cols = split("\t", $_);
    my $sentNo=$cols[0];
    my $altNo=$cols[1];
    my $score = $cols[scalar(@cols)-1];
    $sentsScores[$sentNo]->{$altNo} = $score;
    $coverage[$sentNo]->{$altNo} = $cols[$coverageCol-1] if (defined($coverageCol));
}
close(F);

#
# $best[sentNo] = alternativeNo
# $count{alternativeNo} = frequency of alternativeNo selected as best
# $total = total number of sentences
#
my @best;
my %count;
my $total=0;
for (my $sentNo=0; $sentNo<scalar(@sentsScores); $sentNo++) {
    my $scores = $sentsScores[$sentNo];
    my @sorted  = sort { $scores->{$b} <=> $scores->{$a} } (keys %$scores);
    $best[$sentNo] = $sorted[0];
    $count{$sorted[0]}++;
    $total++;
}



open(F, "<:encoding(utf-8)", $inputFile) or die "Cannot open '$inputFile'";
my $sentNo=-1;
my $printing=0;
my @columnsData; # data[sentNo]->{seqNo}->[lineNo]->[colNo] = content of column
my @confidence; # confidence[sentNo]->{seqNo} = confidence score
my ($num, $confi);
while (<F>) {
    chomp;
    if (!m/^\s*$/) {
	if (m/^# \d/) { # line of the form '# ' = start of a new alternative sequence
	    my $line = $_;
 	    ($num, $confi) = ($line =~ m/^# (\d)\s+(.*)\s*$/); # get alternative no
	    if ($num==0) { #incrementing sentNo if first alternative, i.e. beginniing of a group of 10 for the new sentence
		$sentNo++;
	    }
	    $confidence[$sentNo]->{$num} = $confi;
	    if ($best[$sentNo] == $num) { # if  current alternative is found the best by weka score
		$printing = 1;
		print OUT "# 0 0.9999999\n"; # replace with 0 for Alfredo's script
	    } else {
		$printing = 0;
	    }
	} else {
	    my $line = $_;
	    # store columns data (for analysis later)
	    my @cols = split("\t", $line);
	    push(@{$columnsData[$sentNo]->{$num}}, \@cols);
	    # write output if best sequence 
	    if ($printing == 1) {
		print OUT "$_\n";
	    }
	}
    } else {
	print OUT "\n";
    }
    
}
close(F);
close(OUT);


if (defined($analysisOutput)) {

    my $externIndexesGold = readExternalGoldFile($goldFileAnalysis)  if (defined($goldFileAnalysis));
    my @goldSeq; # goldSeq[sentNo]->{seqNo} = 1 if alternative seqNo corresponds to gold labelling (remark: there can be zero or several for one sentence)
    my @indexesExprs; # $indexesExprs[sentNo]->{gold|crfTop|semTop}->[exprNo]->[wordExprNo] = indexInSentence
    my @words; # $words[sentNo]->[indexInSentence] = word

    # analysis 1: get info by sequence
    for (my $sentNo=0; $sentNo < scalar(@columnsData); $sentNo++) {
#	print STDERR "DEBUG: sentNo=$sentNo\n";
	my @sentGold;
	my @predExprs;
	my @sentWords;
	foreach my $seqNo (sort keys %{$columnsData[$sentNo]}) {
	    my $seqData = $columnsData[$sentNo]->{$seqNo};
#	    print STDERR "  DEBUG: seqNo=$seqNo\n";
	    my @currentSeqPred = ();
	    for (my $lineNo=0; $lineNo<scalar(@$seqData); $lineNo++) {
		my $cols = $seqData->[$lineNo];
#		print STDERR "  DEBUG: lineNo=$lineNo; cols = ".join(";",@$cols)."\n";
		if ($seqNo == 0) { # sentence-level stuff done only for first sequence
		    push(@sentWords, $cols->[$tokenOrLemmaCol]);
		    if (defined($goldFileAnalysis)) {
			# this is a sanity check to make sure we match the right corresponding line in the gold file
			# update: this sanity check proved useful ;)
			my $s1 = $cols->[$tokenOrLemmaCol];
			my $s2 = $externIndexesGold->[$sentNo]->[$lineNo]->[$tokenOrLemmaCol];
			my @gcols = @{$externIndexesGold->[$sentNo]->[$lineNo]};
#			print STDERR "  DEBUG GOLD: lineNo=$lineNo; cols = ".join(";",@gcols)."\n";
			die "BUG token or lemma dont match between input ('$s1') and gold file ('$s2'), line $lineNo: ." if ($s1 ne $s2);
			# caution: the gold BIO file has only one column for labels (8 columns total), but it's the same col no for gold
#
			push(@sentGold, $externIndexesGold->[$sentNo]->[$lineNo]->[$goldCol]);
		    } else { # assuming the gold in the file:
			push(@sentGold, $cols->[$goldCol]);
		    }
		}
		push(@currentSeqPred, $cols->[$predCol]);
	    }
	    $predExprs[$seqNo] = exprIndexes(\@currentSeqPred);
	    if ($seqNo ==0) {  # sentence-level stuff done only for first sequence
		$indexesExprs[$sentNo]->{gold} = exprIndexes(\@sentGold);
		$indexesExprs[$sentNo]->{crfTop} = $predExprs[$seqNo]; # seq 0 is the top CRF prediction
	    }
	    if (identicalExprs($predExprs[$seqNo], $indexesExprs[$sentNo]->{gold})) {
		$goldSeq[$sentNo]->{$seqNo} = 1;
	    }
	}
	push(@words, \@sentWords);
	$indexesExprs[$sentNo]->{semTop} = $predExprs[$best[$sentNo]];
	
    }
    open(OUT, ">:encoding(utf-8)", $analysisOutput) or die "Cannot open '$analysisOutput'";
    print OUT "sentNo\tcrfGoldSeqNo\tsemanticSelectionSeqNo\tcrfTopConfidence\tcrfGoldSeqConfidence\tsemanticSelectionConfidence\tcrfTopSemanticScore\tsemanticSelectionScore\tsemanticGoldScore\tgoldNbExprs\tgoldContinuousProp\tgoldExpressions\tcrfNbExprs\tcrfContinuousProp\tcrfExpressions\tsemanticNbExprs\tsemanticContinuousProp\tsemanticExpressions";
    if (defined($coverageCol)) {
	print OUT "\tgoldCoverage\tcrfCoverage\tsemanticCoverage\n";
    } else {
	print OUT "\n";
    }
    for  (my $sentNo=0; $sentNo < scalar(@columnsData); $sentNo++) {
#	my $goldNbExprs = scalar(@{$goldExprs[$sentNo]});
	my $crfTopConf = $confidence[$sentNo]->{0};
	my $semanticConf = $confidence[$sentNo]->{$best[$sentNo]};
	my ($crfGoldSeqNo, $crfGoldSeqConfidence, $semGoldScore)  = ($nanStr, $nanStr, $nanStr); # default value if the gold is not in any of the 10 sequences
	if (scalar(keys %{$goldSeq[$sentNo]})>0) {
	    my @sortedGoldSeqs = sort { $a <=> $b } (keys %{$goldSeq[$sentNo]});
	    $crfGoldSeqNo = $sortedGoldSeqs[0];
	    $crfGoldSeqConfidence = $confidence[$sentNo]->{$crfGoldSeqNo};
	    $semGoldScore = $sentsScores[$sentNo]->{$crfGoldSeqNo};
	}
	my $crfTopSemanticScore = $sentsScores[$sentNo]->{0};
	my $semSelecScore = $sentsScores[$sentNo]->{$best[$sentNo]};
	my $goldExprs = exprsAsString($indexesExprs[$sentNo]->{gold}, $words[$sentNo], "\t");
	my $crfExprs = exprsAsString($indexesExprs[$sentNo]->{crfTop}, $words[$sentNo], "\t");
	my $semExprs = exprsAsString($indexesExprs[$sentNo]->{semTop}, $words[$sentNo], "\t");
	print OUT "$sentNo\t$crfGoldSeqNo\t$best[$sentNo]\t$crfTopConf\t$crfGoldSeqConfidence\t$semanticConf\t$crfTopSemanticScore\t$semSelecScore\t$semGoldScore\t$goldExprs\t$crfExprs\t$semExprs";
	if (defined($coverageCol)) {
	    my $covGold = ($crfGoldSeqNo eq $nanStr) ?  $nanStr : $coverage[$sentNo]->{$crfGoldSeqNo} ;
	    print OUT "\t".$covGold."\t".$coverage[$sentNo]->{0}."\t".$coverage[$sentNo]->{$best[$sentNo]}."\n";
	} else {
	    print OUT "n";
	}
    }
    close(OUT);


}



$sentNo++;
if ($sentNo != scalar(@sentsScores)) {
    print STDERR "Error: sentNo=$sentNo but sentsScores contains ".scalar(@sentsScores)." sentences\n";
    exit 1;
}

foreach my $n (sort keys %count) {
    printf("$n: $count{$n} (%7.2f%%)\n",$count{$n}*100/$total);
}
