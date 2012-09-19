use strict;
use warnings;

use Term::ANSIColor;
use Graph::Undirected;
# use Graph::Writer::Dot;
use Algorithm::Combinatorics qw(combinations);
use List::Util qw(sum min max);
use POSIX;
use List::Uniq ':all';
use Data::Dumper; 

use Algorithm::Evolutionary qw(
	Individual::BitString
	Op::Breeder
	Op::Mutation
	Op::Crossover
	Op::RouletteWheel
);
use Algorithm::Evolutionary::Utils qw(
	average
	decode_string
);

my $w = shift || 22-2;
my $h = shift || 16-2;
my $space_bits = $w*$h;

my @entry = ( [int($w/3),0], [int($w*2/3),0], [0,$h-1], [int($w/2),$h-1], [$w-1,$h-1],); # 3 to 2
# my @entry = ( [$w-1,$h-1], [0,0], [int($w/2),0], [$w-1,0],); # 1 to 3

# my @entry = ( [int($w/2),0], [0,$h-1], [int($w/2),$h-1], [$w-1,$h-1],); # 3 doors on side one another
# my @entry = ( [int($w/2),0], [int($w/2),$h-1], ); # one door on each side
# my @entry = ( [int($w/2),0], [$w-1,int($h/2)], ); # one door in front other on side
# my @entry = ( [int($w/2),0], [$w-1,int($h/2)], [int($w/5),0], [$w-1,int($h/5)], ); # two doors in front other two on side

my @task = combinations([@entry],2);

my $elitism = 0.05;

my $min_pop_size = shift || 10;
my $max_pop_size = shift || 50;
my $min_mut_rate = shift || .2;
my $max_mut_rate = shift || .9;
my $min_cross_section = shift || 2;
my $max_cross_section = shift || 16;

my $mut_bits = 16;
my $cross_bits = 4;
my $pop_bits = 8;
my $ga_bits = $mut_bits + $cross_bits + $pop_bits;

my $length_pow = 3;
my $coverage_pow = 6;
my $openness_pow = 1;
my $overload_item_pow = 2;
my $overload_pow = 2;
my $dissonance_item_pow = 2;
my $dissonance_pow = 1;

my $fit_p_max = shift || 8;
my $fit_p_bits = 8;
my $fit_bits = 5*$fit_p_bits;

my $bit_count = $space_bits+$ga_bits; # +$fit_bits;


# my $writer = Graph::Writer::Dot->new();

my @pop = (
	Algorithm::Evolutionary::Individual::BitString->fromString( '0' x $bit_count ),
);
while (@pop < $min_pop_size) {
	push @pop, Algorithm::Evolutionary::Individual::BitString->new($bit_count);
}
# use Data::Dumper; print Dumper \@pop;

package  Algorithm::Evolutionary::Op::RouletteWheel::Fix;
our @ISA = qw(Algorithm::Evolutionary::Op::RouletteWheel);

sub apply ($$)
{
  my $self = shift;
  my $pop = shift;
  $self->SUPER::apply(@$pop);
}

package main;


my $mutator =  Algorithm::Evolutionary::Op::Mutation->new($min_mut_rate);
my $crossover = Algorithm::Evolutionary::Op::Crossover->new($min_cross_section);
my $selector = Algorithm::Evolutionary::Op::RouletteWheel::Fix->new($min_pop_size);

my $generation = Algorithm::Evolutionary::Op::Breeder->new( [$mutator, $crossover], $selector );


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
		# avoid camper points?
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
				my $char = $grid[$x][$y] ? '###' : sprintf "%03x", $visit;
				print color($color), $char;
			}
			print color('reset'), "\n";
		}

		# $writer->write_graph($graph, 'graph.dot');
	}

	my $openness = $space_bits;
	for (my $y = 0; $y < $h; $y++){
		for (my $x = 0; $x < $w; $x++){
			$openness-- if $grid[$x][$y];
		}
	}

	$openness /= $space_bits;

	# return 1 if @path != @task || grep {!$_} @path;
	my $should = @task;
	my $did = grep {$_} @path;
	return 1+
			5*$openness+
			10*$did/$should 
				if $should != $did;

	my $n = @path;
	my $length = sum(@path);
	my $average = $length/$n;
	$length /= $space_bits;

	my @dissonance = map { abs($_ - $average) ** $dissonance_item_pow } @path;
	my $pow_sum = sum(map { $_ ** $dissonance_item_pow } @path);
	my $dissonance = 1+sum(@dissonance) / $pow_sum ;

	my $overload_max = 1;
	my $coverage_max = 1;
	my $overload = 1;
	my $coverage = 1;
	for (my $y = 0; $y < $h; $y++){
		for (my $x = 0; $x < $w; $x++){
			my $visit = exists $visited[$x][$y] ? $visited[$x][$y] : 0;
			my $over = unpack '%b*', pack 'I', $visit;
			
			$overload += $over ** $overload_item_pow;
			$overload_max += scalar(@task) ** $overload_item_pow;

			$coverage ++ if $over;
			$coverage_max ++;
		}
	}
	$overload /= $overload_max;
	$coverage /= $coverage_max;

	my $score = 100 + 
		$coverage ** $coverage_pow 
		* $length ** $length_pow 
		/ $dissonance  ** $dissonance_pow
		/ $overload  ** $overload_pow
		/ $openness ** $openness_pow
	;

	# print "score $score\n" if $print;

	return $score;
};

my @elite = ();
my $genCount=0;
while(1) {
	print '-' x 60, "\n";
	
	@pop = 
		sort { $b->Fitness() <=> $a->Fitness() }
		map { $_->Fitness() || $_->evaluate( $fitness ); $_ }
		@pop;

	my $elite = min(int($min_pop_size/4), scalar(@pop));
	my @hist = (@elite, @pop);
	@hist = @{uniq({sort=>1, compare => sub { $_[1]->Fitness() <=> $_[0]->Fitness() }}, \@hist )};
	@elite = @hist[0..min($elite,scalar(@hist))-1];
	$elite = scalar(@elite);

	my ($mut, $cross, $sel) = (0,0,0);
	my ($plen, $pcov, $popen, $pdis, $pover) = (0,0,0,0,0);
	for(@pop) {
		$mut   += (decode_string(substr($_->as_string(),$space_bits,                       $mut_bits),   $mut_bits,   $min_mut_rate,      $max_mut_rate      ))[0];
		$cross += (decode_string(substr($_->as_string(),$space_bits+$mut_bits,             $cross_bits), $cross_bits, $min_cross_section, $max_cross_section ))[0];
		$sel   += (decode_string(substr($_->as_string(),$space_bits+$mut_bits+$cross_bits, $pop_bits),   $pop_bits,   $min_pop_size,      $max_pop_size      ))[0];

#		$plen  += (decode_string(substr($_->as_string(),$space_bits+$ga_bits+$fit_p_bits*0, $fit_p_bits), $fit_p_bits, 0, $fit_p_max ))[0];
#		$pcov  += (decode_string(substr($_->as_string(),$space_bits+$ga_bits+$fit_p_bits*1, $fit_p_bits), $fit_p_bits, 0, $fit_p_max ))[0];
#		$popen += (decode_string(substr($_->as_string(),$space_bits+$ga_bits+$fit_p_bits*2, $fit_p_bits), $fit_p_bits, 0, $fit_p_max ))[0];
#		$pdis  += (decode_string(substr($_->as_string(),$space_bits+$ga_bits+$fit_p_bits*3, $fit_p_bits), $fit_p_bits, 0, $fit_p_max ))[0];
#		$pover += (decode_string(substr($_->as_string(),$space_bits+$ga_bits+$fit_p_bits*4, $fit_p_bits), $fit_p_bits, 0, $fit_p_max ))[0];
	}
#	$length_pow     = $plen  / @pop;
#	$coverage_pow   = $pcov  / @pop;
#	$openness_pow   = $popen / @pop;
#	$dissonance_pow = $pdis  / @pop;
#	$overload_pow   = $pover / @pop;

	$mutator->{mutRate} = $mut / @pop;
	$crossover->{numPoints} = int $cross / @pop;
	my $total_pop = $selector->{_outputSize} = int $sel / @pop;

	print "gen $genCount\n";
	print "mut $mutator->{mutRate}; cross $crossover->{numPoints}; pop $selector->{_outputSize}\n";
	print "len $length_pow; cov $coverage_pow; open $openness_pow; dis $dissonance_pow; over $overload_pow\n";
	print "average fitness: ",average( \@pop ),"\n";
	
	print "all time top:\n";
	for(@elite[0..1]) {
		&$fitness($_, 1);
	}
	print "generation top:\n";
	&$fitness($pop[0], 1);

	my $new_pop = $generation->apply( rand() < $elitism ? \@hist : \@pop );


	@pop = @$new_pop[0..min($total_pop,scalar(@$new_pop))-1]; # no elitism

	while (@pop < $total_pop) {
		push @pop, Algorithm::Evolutionary::Individual::BitString->new($bit_count);
	}

	my $actual_pop_size = scalar @pop;

	my $commoner = min($total_pop - $elite, scalar(@$new_pop));
	print "$actual_pop_size <= $total_pop = $elite + $commoner\n";

	$genCount++;
}

