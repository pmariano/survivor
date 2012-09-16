#!/usr/bin/perl

#------------------------------------------------------------------------
#    Copyright (C) 2011 Luca Amore <luca.amore at gmail.com>
#
#    Maze is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    Maze is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Maze.  If not, see <http://www.gnu.org/licenses/>.
#------------------------------------------------------------------------

use strict;
use warnings;

package Maze;

use Carp qw(croak verbose);
use GD;

sub new {
    my ($class, $x, $y) = @_; 

    my $self = { 
        x => $x, 
        y => $y,
        door => [], 
        solution => [],
    };

    bless $self;
    $self->openWall(0, 0, 'N');
    $self->openWall($x - 1, $y - 1, 'S');

    return $self;
}

sub _getWallIndex($$$){
    my ($self, $x, $y, $dir) = @_;

    my ($wx, $wy, $wdir) = 
        $dir eq 'N' ? ($x,      $y,     0) :
        $dir eq 'S' ? ($x,      $y + 1, 0) :
        $dir eq 'W' ? ($x,      $y,     1) :
        $dir eq 'E' ? ($x + 1,  $y,     1) :
            croak "wrong direction";

    return ($wy*$self->{x}+$wx)*2+$wdir;
}

sub isWallOpen($$$){
    my ($self, $x, $y, $dir) = @_;

    my $idx = $self->_getWallIndex($x, $y, $dir);    

    return $self->{door}[$idx] || 0;
}

sub openWall($$$) {
    my ($self, $x, $y, $dir) = @_;

    my $idx = $self->_getWallIndex($x, $y, $dir);    

    $self->{door}[$idx] = 1;
}

sub getCellNeighbors($$$){
    my ($self, $x, $y) = @_;
    grep {
        $_->[0] >= 0 and $_->[0] < $self->{x} and
        $_->[1] >= 0 and $_->[1] < $self->{y}
    } (
        [$x - 1, $y    , 'W'],
        [$x + 1, $y    , 'E'],
        [$x    , $y - 1, 'N'],
        [$x    , $y + 1, 'S']
    );
}

sub getCellOpenedNeighbors($$$){
    my ($self, $x, $y) = @_;
    grep { 
        my (undef, undef, $dir) = @$_;
        $self->isWallOpen($x, $y, $dir)
    } $self->getCellNeighbors($x, $y);
}

sub isCellExit($$$){
    my ($self, $x, $y) = @_;
    return 
        ($x == $self->{x} -1 ) && ($y == $self->{y} -1 );
} 

sub markSolution($$$$){
    my ($self, $x, $y, $id) = @_;
    $self->{solution}[$x][$y] |= 1 << $id;
}

sub isSolution($$$){
    my ($self, $x, $y) = @_;
    $self->{solution}[$x][$y];
}

# generate maze
sub asterione($$$;$){
    no warnings 'recursion';
    my ($self, $x, $y, $x1, $y1, $visited) = @_;
    $visited->[$x][$y] = 1;
    return if $x == $x1 && $y == $y1;
    my @neighbors = $self->getCellNeighbors($x, $y);
    while (scalar @neighbors){
        my ($tox, $toy, $dir) = 
            @{ splice(@neighbors, rand(@neighbors), 1) };
        next if $visited->[$tox][$toy];
        $self->openWall($x, $y, $dir);
        $self->asterione($tox, $toy, $x1, $y1, $visited);
    }
}

# solve maze
sub teseo($$$$$;$){
    no warnings 'recursion';
    my ($self, $x, $y, $x1, $y1, $id, $visited) = @_;
    $visited->[$x][$y] = 1;
    # if ($self->isCellExit($x, $y)){
    if ($x == $x1 && $y == $y1){
        $self->markSolution($x, $y, $id);
        return 1;
    }
    my @neighbors = $self->getCellOpenedNeighbors($x, $y);
    while (scalar @neighbors){
        my ($tox, $toy, $dir) = 
            @{ splice(@neighbors, rand(@neighbors), 1) };
        next if $visited->[$tox][$toy];
        my $isSolution = $self->teseo($tox, $toy, $x1, $y1, $id, $visited);
        if ($isSolution){
            $self->markSolution($x, $y, $id);
            return $isSolution+1;
        }
    }
    return 0;
}

use Term::ANSIColor;

sub toText(){

    my $self = shift;

    my ($x, $y, @l1, @l2);
    my $out = '';
    for ($y = 0; $y < $self->{y}; $y++){
        @l1 = @l2 = ();
        for ($x = 0; $x < $self->{x}; $x++){
            my $sol = $self->isSolution($x, $y) || 0;
			my $overload = unpack '%b*', pack 'I', $sol;
			my @color = qw(black blue cyan green yellow red magenta);
            my $solution = color($color[$overload]) .
				sprintf("%02x", $sol). 
				color('reset');
            push(@l1, $self->isWallOpen($x, $y, 'N') ? $solution : '-' x 2);
            push(@l2, $self->isWallOpen($x, $y, 'W') ? ' ' : '|');
            push(@l2, $solution);
        }
        push(@l2, $self->isWallOpen($x, $y, 'E') ? ' ' : '|');
        $out .= 
            '+' . join('+',@l1) . '+' . "\n" . 
            join('',@l2) . "\n";
    }

    @l1 = ();
    for ($x = 0; $x < $self->{x}; $x++){
        my $solution = $self->{solution}[$x][$self->{y} -1] ? '.' : ' ';
        push(@l1, 
            $self->isWallOpen($x, $self->{y} -1, 'S') ? 
                $solution x 2 : '-' x 2
        );
    }

    $out .= '+' . join('+', @l1) . '+' . "\n";

    print $out;
}

sub toImage($$){

    my ($self, $FILENAME) = @_;

    my ($WX, $WY) = (10, 10);

    my ($SIZEX, $SIZEY) = ($self->{x} * $WX, $self->{y} * $WY);

    my $img = new GD::Image->newTrueColor($SIZEX,$SIZEY)
        or croak "Can't create GD::Image";
 
    my $cl_white = $img->colorAllocate(255,255,255);
    my $cl_black = $img->colorAllocate(  0,  0,  0);
    my $cl_red   = $img->colorAllocate(255,  0,  0);
    
    $img->fill( 0, 0, $cl_white);

    open(my $fh, '>', $FILENAME)
        or croak "Can't open $FILENAME: $!";

    binmode $fh;

    my ($xx, $yy);

    YY: for ($yy = 0; $yy < $self->{y}; $yy++){

        XX: for ($xx = 0; $xx < $self->{x}; $xx++){

            $img->filledRectangle(
                $xx * $WX, $yy * $WY, ($xx + 1) * $WX, ($yy + 1) * $WY, 
                $cl_red
            )
                if $self->isSolution($xx, $yy);
            
            $img->line(
                    $xx * $WX, $yy * $WY, ($xx + 1) * $WX, $yy * $WY, 
                    $cl_black
            )
                unless $self->isWallOpen($xx, $yy, 'N');

            $img->line(
                    $xx * $WX, $yy * $WY, $xx * $WX, ($yy + 1) * $WY, 
                    $cl_black
            )
                unless $self->isWallOpen($xx, $yy, 'W');
        }

        $img->line(
            $xx * $WX - 1, $yy * $WY, $xx * $WX -1, ($yy + 1) * $WY, 
            $cl_black
        )
            unless $self->isWallOpen($xx - 1, $yy, 'E');
    }

    for ($xx = 0; $xx < $self->{x}; $xx++){
        $img->line(
            $xx * $WX, $yy * $WY - 1, ($xx + 1) * $WX, $yy * $WY - 1, 
            $cl_black
    )
                unless $self->isWallOpen($xx, $yy - 1, 'S');
    }

    print $fh $img->png(0);
    close $fh;
}

package main;




use Algorithm::Combinatorics qw(combinations);
use Storable qw(dclone);
use List::Util qw(sum);

use Algorithm::Evolutionary qw( Individual::BitString
				Op::CanonicalGA Op::Bitflip 
				Op::Crossover Fitness::Royal_Road);
use Algorithm::Evolutionary::Utils qw(entropy consensus);

my $pop_size = shift || 100;
my $selection_rate = shift || 0.25;

my $w = 10;
my $h = 7;

my @entry = (
	[$w/2,0],
	[0,$h-1],
	[$w/2,$h-1],
	[$w-1,$h-1],
);

my @task = combinations([@entry],2);
# print join ';', map {join ',', map { "@$_"}  @$_} combinations([@entry],2);

my $max_score = -1;
my $winner = {};

my $bit_count = $w*$h*2;
my @pop;
for (1..$pop_size) {
	push @pop, Algorithm::Evolutionary::Individual::BitString->new($bit_count);

}
# use Data::Dumper; print Dumper \@pop;

my $m = Algorithm::Evolutionary::Op::Bitflip->new(1);
my $c = Algorithm::Evolutionary::Op::Crossover->new(2,4);

my $fitness = sub {
	my $indi = shift;
	my $print = shift;

	my @door = map {$indi->Atom($_)} 0..$indi->size()-1;

	my $map = Maze->new($w,$h);
	$map->{door} = \@door;

	my $id = 0;
	my @path = map {
		my ($start, $end) = @$_;
		$map->teseo(
				$start->[0], $start->[1],
				$end->[0], $end->[1],
				$id++,
				);
	} @task;

	return 1 if @path != @task || grep {!$_} @path;

	my $n = @path;
	my $sum = sum(@path);
	my $average = $sum/$n;
	my $dissonance_pow = 2;
	my @dissonance = map { abs($_ - $average) ** $dissonance_pow } @path;
	my $pow_sum = sum(map { $_ ** $dissonance_pow } @path);
	my $dissonance = sum(@dissonance) / $pow_sum ;

	my $overload = 1;
	my $coverage = 0;
	for (my $y = 0; $y < $h; $y++){
		for (my $x = 0; $x < $w; $x++){
			my $sol = $map->isSolution($x, $y) || 0;
			my $over = unpack '%b*', pack 'I', $sol;
			$overload += $over ** 2;
			$coverage ++ if $over;
		}
	}

	my $score = 10 + $coverage ** 3 * $sum ** 2 / $dissonance / $overload ;

	if($print) {
		print $indi->asString()," $score \n";
		print "path lengths @path\n";
		$map->toText();
	}

	return $score;
};

map( $_->evaluate( $fitness ), @pop ); 

my $generation = Algorithm::Evolutionary::Op::CanonicalGA->new( $fitness , $selection_rate , [$m, $c] ) ;

my $genCount=0;
while(1) {
	$generation->apply( \@pop );
	print "$genCount\n";

	&$fitness($pop[0], 1);
	&$fitness($pop[0], 1);

	$genCount++;
}

