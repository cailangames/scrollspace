// A program to generate biomes (map levels) for ScrollSpace
//
// To run this program, simply run the following command:
//
//	go run generate_biomes.go
package main

import (
	"fmt"
	"log"
	"math/rand"
	"time"
)

// Tunable constants
const (
	// To use a specific seed for the RNG, set `randomSeed` to a non-zero value.
	randomSeed int64 = 0
)

// Note: These constants should match the constant values in the C code.
const (
	biomeCount      = 64
	columnsPerBiome = 20
	rowsPerColumn   = 17
	minCaveWidth    = 3
	maxCaveWidth    = 10
)

var rng *rand.Rand

// Combines the data for a column into one uint8.
func combineColumnData(topRow uint8, width uint8) uint8 {
	var col uint8 = topRow << 4
	col |= width
	return col
}

func topRowDelta() int {
	n := rng.Intn(256)
	if n < 24 {
		return 0
	}
	if n < 140 {
		return -2
	}
	return 2
}

func widthDelta() int {
	n := rng.Intn(256)
	if n < 24 {
		return -1
	}
	if n < 140 {
		return 1
	}
	return 0
}

// Generates the biomes. The first dimension of the returned [][]uint8 is the biomes themselves.
// The second dimension is the column data per biome.
func generateBiomes() [][]uint8 {
	biomes := make([][]uint8, biomeCount)
	prevTopRow := 7
	prevWidth := 5
	for i := range biomes {
		biomes[i] = make([]uint8, columnsPerBiome)
		for j := range biomes[i] {
			topRow := max(prevTopRow+topRowDelta(), 0)
			topRow = min(topRow, rowsPerColumn-minCaveWidth)
			wDelta := widthDelta()
			width := max(prevWidth+wDelta, minCaveWidth)
			width = min(width, maxCaveWidth)
			width = min(width, rowsPerColumn-topRow)
			// In order to allow for more width values in this 4-bit integer, we subtract minCaveWidth
			// and later add it back in the game code.
			subWidth := width - minCaveWidth
			if subWidth < 0 {
				// This should only happen if there's a bug in this code.
				log.Fatalf("Invalid width! width=%d, prevWidth=%d, delta=%d, topRow=%d", width, prevWidth, wDelta, topRow)
			}
			biomes[i][j] = combineColumnData(uint8(topRow), uint8(subWidth))
			prevTopRow = topRow
			prevWidth = width
		}
	}
	return biomes
}

// Prints the biome data to stdout in the C code format.
func printBiomes(biomes [][]uint8) {
	fmt.Println("static const uint8_t biome_columns[BIOME_COUNT][COLUMNS_PER_BIOME] = {")
	for _, biome := range biomes {
		fmt.Printf("  { ")
		first := true
		for _, col := range biome {
			if first {
				fmt.Printf("0x%02X", col)
				first = false
				continue
			}
			fmt.Printf(", 0x%02X", col)
		}
		fmt.Println(" },")
	}
	fmt.Println("};")
}

func main() {
	fmt.Println("Generating biomes...")
	seed := randomSeed
	if seed == 0 {
		seed = time.Now().UnixNano()
	}
	rng = rand.New(rand.NewSource(seed))
	fmt.Printf("Using random seed: %d\n", seed)
	biomes := generateBiomes()
	fmt.Printf("Generated biome count: %d\n", len(biomes))
	printBiomes(biomes)
}
