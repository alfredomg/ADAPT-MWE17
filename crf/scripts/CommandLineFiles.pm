package CommandLineFiles;
$VERSION = v0.0.1;

use warnings;
use strict;
use File::Spec;

require Exporter;
our @ISA = qw(Exporter);
# Export by default:
our @EXPORT = qw(
    GetFilesFromCommandLine
);

sub GetFilesFromCommandLine
{
	my $getrelativepaths = shift;
	my $sort_in_order_of_appearance = shift;

	my $TotalFiles = @ARGV;
	if ($TotalFiles <= 0)
	{
		print STDERR "\nSpecify files!\n\n";
	    exit 1;
	}
	my %FilesH = ();
	my $order = 1;
	foreach my $item (@ARGV)
	{
		if (($item =~ /\*/) or ($item =~ /\?/))
		{
			my @list = glob $item;
			foreach my $i (@list)
			{
				$FilesH{$i} = $order;
			}
		}
		else
		{
			$FilesH{$item} = $order;
		}
		$order++;
	}

	my @Files;
	if ($getrelativepaths)
	{
		 # Relative paths are OK
		 @Files = sort(keys %FilesH);
	}
	else
	{
		# Get absolute path to files (assuming we have paths relative to current directory) - default behaviour
		@Files = ();
		my @sorted_files = $sort_in_order_of_appearance ? sort {$FilesH{$a} <=> $FilesH{$b}} (keys %FilesH) : sort (keys %FilesH);
		foreach my $filename (@sorted_files)
		{
			my $abs_path = ($filename !~ /^~/) ? File::Spec->rel2abs($filename) : $filename;
			# noticed that if ~ is the first char in the path, rel2abs doesn't work. So if ~ is first char in path, we leave it like that as that's some sort of absolute path.
			push @Files, $abs_path;
		}
	}


	return @Files;
}

1;
