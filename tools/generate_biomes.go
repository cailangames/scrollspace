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

// Parameters
const (
	// To use a specific seed for the RNG, set `randomSeed` to a non-zero value.
	randomSeed int64 = 0
)

// Constants
const (
	biomeCount      = 64
	columnsPerBiome = 20
	rowsPerColumn   = 17
	minCaveWidth    = 3
)

var rng *rand.Rand

// Returns the nearest power of 2 that's greater than or equal to `n`.
// Note: This function only goes up to 64.
func nearestPowerOf2(n int) int {
	switch {
	case n <= 0:
		return 0
	case n <= 1:
		return 1
	case n <= 2:
		return 2
	case n <= 4:
		return 4
	case n <= 8:
		return 8
	case n <= 16:
		return 16
	case n <= 32:
		return 32
	case n <= 64:
		return 64
	default:
		log.Fatalf("Input to nearestPowerOf2() too large: %d", n)
		return 0
	}
}

// Packs the data for a column into one uint8.
func packColumnData(topRow uint8, width uint8) uint8 {
	if topRow > rowsPerColumn-minCaveWidth || width < minCaveWidth || width > rowsPerColumn-topRow {
		// This should only happen if there's a bug in this code.
		log.Fatalf("Invalid column! topRow=%d, width=%d", topRow, width)
	}
	// In order to allow for more width values in this 4-bit integer, we subtract minCaveWidth
	// and later add it back when unpacking the column data.
	width -= minCaveWidth
	var col uint8 = topRow << 4
	col |= width
	return col
}

// Unpacks the data for a column and returns the top row and width of the cave.
func unpackColumnData(data uint8) (uint8, uint8) {
	var topRow uint8 = data >> 4
	var width uint8 = data & 0x0F
	return topRow, width + minCaveWidth
}

type Generator interface {
	Generate() int
}

type BiomeGenerator struct {
	startingTopRow int
	startingWidth  int
	topRowDelta    Generator
	widthDelta     Generator
}

func highTopRow() int {
	return rng.Intn(5)
}

func middleTopRow() int {
	return rng.Intn(5) + 5
}

func lowTopRow() int {
	return rng.Intn(5) + 10
}

func narrowWidth() int {
	n := rng.Intn(100) + 1
	switch {
	case n <= 50:
		return 4
	case n <= 75:
		return 3
	default:
		return 5
	}
}

func standardWidth() int {
	n := rng.Intn(100) + 1
	switch {
	case n <= 35:
		return 6
	case n <= 70:
		return 7
	case n <= 85:
		return 5
	default:
		return 8
	}
}

func wideWidth() int {
	n := rng.Intn(100) + 1
	switch {
	case n <= 50:
		return 9
	case n <= 75:
		return 8
	default:
		return 10
	}
}

func steadyDelta() int {
	n := rng.Intn(100) + 1
	switch {
	case n <= 20:
		return 0
	case n <= 55:
		return -1
	case n <= 90:
		return 1
	case n <= 95:
		return -2
	default:
		return 2
	}
}

func volatileDelta() int {
	n := rng.Intn(100) + 1
	switch {
	case n <= 10:
		return 0
	case n <= 50:
		return -2
	case n <= 90:
		return 2
	case n <= 95:
		return -1
	default:
		return 1
	}
}

func increasingDelta() int {
	n := rng.Intn(100) + 1
	switch {
	case n <= 50:
		return 1
	case n <= 75:
		return 2
	case n <= 85:
		return 0
	case n <= 95:
		return -1
	default:
		return -2
	}
}

func decreasingDelta() int {
	return increasingDelta() * -1
}

type steadyTopRowDelta struct{}

func (d *steadyTopRowDelta) Generate() int {
	return steadyDelta()
}

type volatileTopRowDelta struct{}

func (d *volatileTopRowDelta) Generate() int {
	return volatileDelta()
}

type risingTopRowDelta struct{}

func (d *risingTopRowDelta) Generate() int {
	return decreasingDelta()
}

type fallingTopRowDelta struct{}

func (d *fallingTopRowDelta) Generate() int {
	return increasingDelta()
}

type steadyWidthDelta struct{}

func (d *steadyWidthDelta) Generate() int {
	return steadyDelta()
}

type volatileWidthDelta struct{}

func (d *volatileWidthDelta) Generate() int {
	return volatileDelta()
}

type wideningWidthDelta struct{}

func (d *wideningWidthDelta) Generate() int {
	return increasingDelta()
}

type narrowingWidthDelta struct{}

func (d *narrowingWidthDelta) Generate() int {
	return decreasingDelta()
}

func newMiddleWideRisingWidening() *BiomeGenerator {
	return &BiomeGenerator{
		startingTopRow: middleTopRow(),
		startingWidth:  wideWidth(),
		topRowDelta:    &risingTopRowDelta{},
		widthDelta:     &wideningWidthDelta{},
	}
}

func clampColumnValues(topRow, width int) (int, int) {
	tr := max(topRow, 0)
	tr = min(tr, rowsPerColumn-minCaveWidth)
	w := max(width, minCaveWidth)
	w = min(w, rowsPerColumn-tr)
	return tr, w
}

// Generates the biomes. The first dimension of the returned [][]uint8 is the biomes themselves.
// The second dimension is the column data per biome.
func generateBiomes() [][]uint8 {
	biomes := make([][]uint8, biomeCount)
	for i := range biomes {
		biomes[i] = make([]uint8, columnsPerBiome)
		gen := newMiddleWideRisingWidening()
		startTop, startWidth := clampColumnValues(gen.startingTopRow, gen.startingWidth)
		biomes[i][0] = packColumnData(uint8(startTop), uint8(startWidth))
		prevTopRow := startTop
		prevWidth := startWidth
		for j := 1; j < len(biomes[i]); j++ {
			topRow, width := clampColumnValues(prevTopRow+gen.topRowDelta.Generate(), prevWidth+gen.widthDelta.Generate())
			biomes[i][j] = packColumnData(uint8(topRow), uint8(width))
			prevTopRow = topRow
			prevWidth = width
		}
	}
	return biomes
}

// Determines if the caves for two biomes overlap.
func biomesOverlap(leftTop, leftBottom, rightTop, rightBottom uint8) bool {
	// left is completely above right.
	if leftBottom < rightTop {
		return false
	}
	// left is completely below right.
	if leftTop > rightBottom {
		return false
	}
	// Otherwise, left and right must overlap.
	return true
}

// Determines if two biomes connect, i.e. their caves overlap with at least the required minimum
// width.
func biomesConnect(leftTop, leftWidth, rightTop, rightWidth uint8) bool {
	leftBottom := leftTop + leftWidth - 1
	rightBottom := rightTop + rightWidth - 1
	if !biomesOverlap(leftTop, leftBottom, rightTop, rightBottom) {
		return false
	}
	// Calculate the actual overlap and see if it's >= the minimum cave width.
	overlapStart := max(leftTop, rightTop)
	overlapEnd := min(leftBottom, rightBottom)
	actualOverlap := overlapEnd - overlapStart + 1
	return actualOverlap >= minCaveWidth
}

// Evenly distributes the given elements in a list of size `n`.
func distributeElements(elements []int, n int) []int {
	if len(elements) > n {
		log.Fatalf("Can't fit %d elements into a list of size %d", len(elements), n)
	}

	result := make([]int, n)
	base := n / len(elements)      // Integer division to get the base distribution
	remainder := n % len(elements) // Remaining elements to distribute
	startIdx := 0
	for _, element := range elements {
		count := base
		if remainder > 0 {
			// Distribute the remainder
			count++
			remainder--
		}
		for i := 0; i < count; i++ {
			result[startIdx+i] = element
		}
		startIdx += count
	}
	return result
}

// Balances the biome connections, i.e. makes every biome's connection list the same size.
func balanceBiomeConnections(connections [][]int, maxCount int) [][]int {
	result := make([][]int, len(connections))
	size := nearestPowerOf2(maxCount)
	for i := range connections {
		result[i] = distributeElements(connections[i], size)
	}
	return result
}

// Finds and returns all biomes that connect. Biomes connect when their caves overlap with at least
// the required minimum width.
func findBiomeConnections(biomes [][]uint8) [][]int {
	connections := make([][]int, len(biomes))
	maxCount := 0
	for leftIdx := range biomes {
		connections[leftIdx] = []int{}
		count := 0
		leftTop, leftWidth := unpackColumnData(biomes[leftIdx][len(biomes[leftIdx])-1])
		for rightIdx := range biomes {
			if rightIdx == leftIdx {
				continue
			}
			rightTop, rightWidth := unpackColumnData(biomes[rightIdx][0])
			if biomesConnect(leftTop, leftWidth, rightTop, rightWidth) {
				connections[leftIdx] = append(connections[leftIdx], rightIdx)
				count++
			}
		}
		if count == 0 {
			log.Fatalf("Biome %d (topRow=%d, width=%d) doesn't connect with any other biome", leftIdx, leftTop, leftWidth)
		}
		maxCount = max(maxCount, count)
	}

	log.Printf("Largest number of connections found: %d", maxCount)
	return balanceBiomeConnections(connections, maxCount)
}

// Prints the constants to stdout in the C code format.
func printConstants(seed int64, biomes [][]uint8, connections [][]int) {
	fmt.Printf("// Random seed used: %d\n", seed)
	fmt.Println()
	fmt.Printf("#define BIOME_COUNT %d\n", len(biomes))
	fmt.Printf("#define BIOME_CONNECTION_COUNT %d\n", len(connections[0]))
	fmt.Printf("#define COLUMNS_PER_BIOME %d\n", columnsPerBiome)
	fmt.Printf("#define MINIMUM_CAVE_WIDTH %d\n", minCaveWidth)
}

// Prints the biome data to stdout in the C code format.
func printBiomes(biomes [][]uint8) {
	fmt.Println("static const uint8_t biome_columns[BIOME_COUNT][COLUMNS_PER_BIOME] = {")
	for i, biome := range biomes {
		fmt.Printf("  /* %02d */ { ", i)
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

// Prints the biome connections to stdout in the C code format.
func printBiomeConnections(connections [][]int) {
	fmt.Println("static const uint8_t next_possible_biomes[BIOME_COUNT][BIOME_CONNECTION_COUNT] = {")
	for i, biome := range connections {
		fmt.Printf("  /* %02d */ { ", i)
		first := true
		for _, conn := range biome {
			if first {
				fmt.Printf("%d", conn)
				first = false
				continue
			}
			fmt.Printf(", %d", conn)
		}
		fmt.Println(" },")
	}
	fmt.Println("};")
}

func main() {
	log.Print("Generating biomes...")
	seed := randomSeed
	if seed == 0 {
		seed = time.Now().UnixNano()
	}
	rng = rand.New(rand.NewSource(seed))
	log.Printf("Using random seed: %d", seed)
	biomes := generateBiomes()
	log.Printf("Generated %d biomes", len(biomes))
	log.Print("Finding biome connections")
	connections := findBiomeConnections(biomes)
	log.Print("Printing biomes and connections")
	printConstants(seed, biomes, connections)
	fmt.Println()
	printBiomes(biomes)
	fmt.Println()
	printBiomeConnections(connections)
	fmt.Println()
}
