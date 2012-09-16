use strict;
use warnings;

use Term::ANSIColor;
use Graph::Undirected;
# use Graph::Writer::Dot;
use Algorithm::Combinatorics qw(combinations);
use List::Util qw(sum);

use Algorithm::Evolutionary qw( Individual::BitString
				Op::CanonicalGA Op::Bitflip 
				Op::Crossover Fitness::Royal_Road);
use Algorithm::Evolutionary::Utils qw(entropy consensus);

my $pop_size = shift || 100;
my $selection_rate = shift || 0.25;

my $w = 22-2;
my $h = 16-2;

my @entry = (
	[$w/2,0],
	[0,$h-1],
	[$w/2,$h-1],
	[$w-1,$h-1],
);

my @task = combinations([@entry],2);
# print join ';', map {join ',', map { "@$_"}  @$_} combinations([@entry],2);

# my $writer = Graph::Writer::Dot->new();

my $bit_count = $w*$h;
my @pop = (
	# Algorithm::Evolutionary::Individual::BitString->fromString( '0' x $bit_count ),
);
for (1..$pop_size) {
	push @pop, Algorithm::Evolutionary::Individual::BitString->new($bit_count);
}
# use Data::Dumper; print Dumper \@pop;

my $m = Algorithm::Evolutionary::Op::Bitflip->new(4);
my $c = Algorithm::Evolutionary::Op::Crossover->new(4);

sub idx {
	my ($x, $y) = @_;
	$x+$y*$w
}

sub coord {
	my ($i) = @_;
	($i % $w, int($i / $w))
}

sub val($$$) {
	my ($ind, $x, $y) = @_;
	$ind->Atom(idx($x,$y))
}

my $fitness = sub {
	my $ind = shift;
	my $print = shift;

	my @grid = ();
	for (my $y=0; $y<$h; $y++) { 
		for (my $x=0; $x<$w; $x++) { 
			$grid[$x][$y] = val($ind,$x,$y);
			# print "$x $y: $grid[$x][$y]\n" if $print;
		}
	}

	my $graph = Graph::Undirected->new;
	for (my $y=0; $y<$h; $y++) { 
		for (my $x=0; $x<$w; $x++) { 
			my $i0 = idx($x,$y);
			my $v0 = $ind->Atom($i0);
			if($x+1<$w) {
				my $i1 = idx($x+1,$y);
				my $v1 = $ind->Atom($i1);
				$graph->add_edge($i0, $i1) unless $v0 || $v1; 
			}

			if($y+1<$h) {
				my $i2 = idx($x,$y+1);
				my $v2 = $ind->Atom($i2);
				$graph->add_edge($i0, $i2) unless $v0 || $v2; 
			}
		}
	}

	my $task_id = 0;
	my @visited = ();
	my @path = map {
		my ($start, $end) = @$_;
		my @path = $graph->SP_Dijkstra(idx(@$start), idx(@$end));
		for my $i (@path) {
			my ($x, $y) = coord($i);
			$visited[$x][$y] |= 1 << $task_id;
		}
		$task_id++;
		scalar(@path)
	} @task;

	if($print) {
		print $ind->asString(), "\n";
		print "path lengths @path\n";
		for (my $y=0; $y<$h; $y++) { 
			for (my $x=0; $x<$w; $x++) { 
				my $visit = exists $visited[$x][$y] ? $visited[$x][$y] : 0;
				my $over = unpack '%b*', pack 'I', $visit;
				my @color = qw(black blue cyan green yellow red magenta);
				my $color = $grid[$x][$y] ? 'white' : $color[$over];
				my $char = $grid[$x][$y] ? '##' : sprintf "%02x", $visit;
				print color($color), $char;
			}
			print color('reset'), "\n";
		}

		# $writer->write_graph($graph, 'graph.dot');
	}

	my $openness = $bit_count;
	for (my $y = 0; $y < $h; $y++){
		for (my $x = 0; $x < $w; $x++){
			$openness-- if $grid[$x][$y];
		}
	}

	# return 1 if @path != @task || grep {!$_} @path;
	my $should = @task;
	my $did = grep {$_} @path;
	return 1+$openness/$bit_count+10*$did/$should if $should != $did;

	my $dissonance_pow = 1;
	my $overload_pow = 1;

	my $n = @path;
	my $sum = sum(@path);
	my $average = $sum/$n;
	my @dissonance = map { abs($_ - $average) ** $dissonance_pow } @path;
	my $pow_sum = sum(map { $_ ** $dissonance_pow } @path);
	my $dissonance = sum(@dissonance) / $pow_sum ;

	my $overload = 1;
	my $coverage = 0;
	for (my $y = 0; $y < $h; $y++){
		for (my $x = 0; $x < $w; $x++){
			my $visit = exists $visited[$x][$y] ? $visited[$x][$y] : 0;
			my $over = unpack '%b*', pack 'I', $visit;
			$overload += $over ** $overload_pow;
			$coverage ++ if $over;
		}
	}

	my $score = 100 + $coverage * $sum / $dissonance / $overload; # / $openness;

	# print "score $score\n" if $print;

	return $score;
};

map( $_->evaluate( $fitness ), @pop ); 

my $generation = Algorithm::Evolutionary::Op::CanonicalGA->new( $fitness , $selection_rate , [$m, $c] ) ;

my $genCount=0;
while(1) {
	$generation->apply( \@pop );
	print "$genCount\n";

	&$fitness($pop[0], 1);

	$genCount++;
}

