#!/usr/bin/env perl

=head1 NAME

colcat.pl -- Concatenate files by column

=head1 SYNOPSIS

 colcat.pl tabfiles.* > concatenated

=head1 DESCRIPTION

Concatenate files by column

Example:

 colcat.pl eval.?.tsv > eval.tsv

=head1 AUTHOR

Alfredo Maldonado (http://alfredomg.com)

=cut

use strict;
use warnings;
use FindBin; # locate this script
use lib "$FindBin::RealBin/."; # include current dir for modules
use CommandLineFiles;

my @tabFiles = GetFilesFromCommandLine(0,1);
my @lines = ();
my $first = 1;
foreach my $tabFile (@tabFiles)
{
    open(my $fo, "<:utf8", $tabFile) or die "Cannot open '$tabFile': $!\n";
    my $ln = 0;
    while (my $line = <$fo>)
    {
        chomp $line;
        if ($first)
        {
            push @lines, $line;
        }
        else
        {
            $lines[$ln] .= "\t" . $line;
        }
        $ln++;
    }
    close($fo);
    $first = 0;
}

foreach my $line (@lines)
{
    print "$line\n";
}



