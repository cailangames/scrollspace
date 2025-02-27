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
	"slices"
	"sort"
	"time"
)

// Parameters
const (
	// To use a specific seed for the RNG, set `randomSeed` to a non-zero value.
	randomSeed int64 = 0
	// How many connections will each biome have. If set to zero, then this value will be
	// algorithmically decided based on how many biome connections exist.
	biomeConnectionCount = 32
	// Biome generator counts
	middleWideRisingWideningCount = 64
	biomeCount                    = middleWideRisingWideningCount
)

// Constants
const (
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

// Holds the data for a column within a biome.
type Column struct {
	TopRow int
	Width  int
}

// Returns a new Column with the values in the given Column clamped to min and max values.
func (c Column) ClampValues() Column {
	tr := max(c.TopRow, 0)
	tr = min(tr, rowsPerColumn-minCaveWidth)
	w := max(c.Width, minCaveWidth)
	w = min(w, rowsPerColumn-tr)
	return Column{
		TopRow: tr,
		Width:  w,
	}
}

// Packs the data for the Column into one uint8.
func (c Column) PackData() uint8 {
	if c.TopRow > rowsPerColumn-minCaveWidth || c.Width < minCaveWidth || c.Width > rowsPerColumn-c.TopRow {
		// This should only happen if there's a bug in this code.
		log.Fatalf("Invalid column! %s", c)
	}
	var topRow uint8 = uint8(c.TopRow)
	// In order to allow for more width values in this 4-bit integer, we subtract minCaveWidth
	// and later add it back when unpacking the column data.
	var width uint8 = uint8(c.Width - minCaveWidth)
	var packed uint8 = topRow << 4
	packed |= width
	return packed
}

func (c Column) String() string {
	return fmt.Sprintf("{TopRow=%d,Width=%d}", c.TopRow, c.Width)
}

// A biome is one unit of the procedural generation output. It's made up of a bunch of columns that
// specify where the blocks (obstacles) are in the biome. Biomes are strung together randomly to
// create a randomly generated game map.
type Biome struct {
	Columns []Column
}

// A Generator is an interface for generating values.
type Generator interface {
	Generate() int
}

// A BiomeGenerator can generate new biomes.
type BiomeGenerator struct {
	StartingTopRow int
	StartingWidth  int
	TopRowDelta    Generator
	WidthDelta     Generator
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

func NewMiddleWideRisingWidening() *BiomeGenerator {
	return &BiomeGenerator{
		StartingTopRow: middleTopRow(),
		StartingWidth:  wideWidth(),
		TopRowDelta:    &risingTopRowDelta{},
		WidthDelta:     &wideningWidthDelta{},
	}
}

// Creates the biome generators based on the program's parameters.
func createBiomeGenerators() []*BiomeGenerator {
	gens := make([]*BiomeGenerator, biomeCount)
	for i := 0; i < middleWideRisingWideningCount; i++ {
		gens[i] = NewMiddleWideRisingWidening()
	}
	return gens
}

// Generates the biomes.
func GenerateBiomes() []*Biome {
	biomes := make([]*Biome, biomeCount)
	gens := createBiomeGenerators()
	for i := range biomes {
		columns := make([]Column, columnsPerBiome)
		gen := gens[i]
		startingCol := Column{
			TopRow: gen.StartingTopRow,
			Width:  gen.StartingWidth,
		}.ClampValues()
		columns[0] = startingCol
		prevTopRow := startingCol.TopRow
		prevWidth := startingCol.Width
		for i := 1; i < len(columns); i++ {
			c := Column{
				TopRow: prevTopRow + gen.TopRowDelta.Generate(),
				Width:  prevWidth + gen.WidthDelta.Generate(),
			}.ClampValues()
			columns[i] = c
			prevTopRow = c.TopRow
			prevWidth = c.Width
		}
		biomes[i] = &Biome{Columns: columns}
	}
	return biomes
}

// Determines if the caves for two columns overlap.
func cavesOverlap(leftTop, leftBottom, rightTop, rightBottom int) bool {
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

// Determines if two columns connect, i.e. their caves overlap with at least the required minimum
// width.
func columnsConnect(leftCol Column, rightCol Column) bool {
	leftTop := leftCol.TopRow
	rightTop := rightCol.TopRow
	leftBottom := leftTop + leftCol.Width - 1
	rightBottom := rightTop + rightCol.Width - 1
	if !cavesOverlap(leftTop, leftBottom, rightTop, rightBottom) {
		return false
	}
	// Calculate the actual overlap and see if it's >= the minimum cave width.
	overlapStart := max(leftTop, rightTop)
	overlapEnd := min(leftBottom, rightBottom)
	actualOverlap := overlapEnd - overlapStart + 1
	return actualOverlap >= minCaveWidth
}

// Shrinks the size of the given []int by randomly picking `n` elements from it.
func shrinkElements(elements []int, n int) []int {
	result := make([]int, n)
	for i := 0; i < len(result); i++ {
		r := rng.Intn(len(elements))
		result[i] = elements[r]
		elements = slices.Delete(elements, r, r+1)
	}
	sort.Ints(result)
	return result
}

// Evenly distributes the given elements in a list of size `n`.
func distributeElements(elements []int, n int) []int {
	if len(elements) > n {
		elements = shrinkElements(elements, n)
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
	size := biomeConnectionCount
	if size <= 0 {
		size = nearestPowerOf2(maxCount)
	}
	resizeCount := 0
	for i := range connections {
		if len(connections[i]) > size {
			resizeCount++
		}
		result[i] = distributeElements(connections[i], size)
	}
	log.Printf("Resized connections list of %d biomes", resizeCount)
	return result
}

// Finds and returns all biomes that connect. Biomes connect when their caves overlap with at least
// the required minimum width.
func FindBiomeConnections(biomes []*Biome) [][]int {
	connections := make([][]int, len(biomes))
	maxCount := 0
	for leftIdx, leftBiome := range biomes {
		connections[leftIdx] = []int{}
		count := 0
		leftCol := leftBiome.Columns[len(leftBiome.Columns)-1]
		for rightIdx, rightBiome := range biomes {
			if rightIdx == leftIdx {
				continue
			}
			rightCol := rightBiome.Columns[0]
			if columnsConnect(leftCol, rightCol) {
				connections[leftIdx] = append(connections[leftIdx], rightIdx)
				count++
			}
		}
		if count == 0 {
			log.Fatalf("Biome %d (%s) doesn't connect with any other biome", leftIdx, leftCol)
		}
		maxCount = max(maxCount, count)
	}

	log.Printf("Largest number of connections found: %d", maxCount)
	return balanceBiomeConnections(connections, maxCount)
}

// Prints the constants to stdout in the C code format.
func PrintConstants(seed int64, biomes []*Biome, connections [][]int) {
	fmt.Printf("// Random seed used: %d\n", seed)
	fmt.Println()
	fmt.Printf("#define BIOME_COUNT %d\n", len(biomes))
	fmt.Printf("#define BIOME_CONNECTION_COUNT %d\n", len(connections[0]))
	fmt.Printf("#define COLUMNS_PER_BIOME %d\n", columnsPerBiome)
	fmt.Printf("#define MINIMUM_CAVE_WIDTH %d\n", minCaveWidth)
}

// Prints the biome data to stdout in the C code format.
func PrintBiomes(biomes []*Biome) {
	fmt.Println("static const uint8_t biome_columns[BIOME_COUNT][COLUMNS_PER_BIOME] = {")
	for i, biome := range biomes {
		fmt.Printf("  /* %02d */ { ", i)
		first := true
		for _, col := range biome.Columns {
			if first {
				fmt.Printf("0x%02X", col.PackData())
				first = false
				continue
			}
			fmt.Printf(", 0x%02X", col.PackData())
		}
		fmt.Println(" },")
	}
	fmt.Println("};")
}

// Prints the biome connections to stdout in the C code format.
func PrintBiomeConnections(connections [][]int) {
	fmt.Println("static const uint8_t next_possible_biomes[BIOME_COUNT][BIOME_CONNECTION_COUNT] = {")
	for i, biome := range connections {
		fmt.Printf("  /* %02d */ { ", i)
		first := true
		for _, conn := range biome {
			if first {
				fmt.Printf("%2d", conn)
				first = false
				continue
			}
			fmt.Printf(", %2d", conn)
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
	biomes := GenerateBiomes()
	log.Printf("Generated %d biomes", len(biomes))
	log.Print("Finding biome connections")
	connections := FindBiomeConnections(biomes)
	log.Print("Printing biomes and connections")
	PrintConstants(seed, biomes, connections)
	fmt.Println()
	PrintBiomes(biomes)
	fmt.Println()
	PrintBiomeConnections(connections)
	fmt.Println()
}
