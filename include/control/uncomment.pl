use strict;
use warnings;

foreach (@ARGV)
{

	open FILE, $_ or die "Couldn't open file: $!"; 
	my $string = join("", <FILE>); 
	close FILE;

	$string=~ s|/[*].*?[*]/||sg;
	$string=~ s/virtual//sg; 
	$string=~ s/=0;/;/sg;
	$string=~ s/(^|\n)[\n\s]*/$1/g;

	open F, ">$_" or die "Couldn't open file: $!"; 
	print F $string;
	close F;
}





