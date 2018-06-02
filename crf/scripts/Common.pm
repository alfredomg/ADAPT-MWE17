package Common;
$VERSION = v0.0.1;

use warnings;
use strict;

require Exporter;
our @ISA = qw(Exporter);
# Export by default:
our @EXPORT = qw(
    countSequences
);

sub countSequences
{
    my $file = shift;

    open(my $fo,   "<:utf8", $file)   or die "Cannot open '$file': $!\n";
    my $nSeqs = 0;
    my $last;
    while (my $line = <$fo>)
    {
	if (!defined $last && $line !~ /^\s*$/)
	{
	    $nSeqs++;
	}
	elsif ($last ne $line && $line =~ /^\s*$/)
	{
	    $nSeqs++;
	}
	
	$last = $line;
    }
    close($fo);
    
    return $nSeqs;
}

1;
