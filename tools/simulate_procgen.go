// A program to simulate a procedurally generated map for ScrollSpace
//
// To run this program, simply run the following command:
//
//	go run simulate_procgen.go
//
// For a more complete example of how to use this and related programs, see the following commands:
//
//	go run generate_biomes.go > biomes.c
//	go run simulate_procgen.go --input_file=biomes.c --output_file=simulation.png
//
// To see more details on how to use this program and its flags, run the following command:
//
//	go run simulate_procgen.go --help
package main

import (
	"flag"
	"fmt"
	"image"
	"image/color"
	"image/png"
	"io/ioutil"
	"log"
	"math"
	"math/rand"
	"os"
	"regexp"
	"strconv"
	"strings"
	"time"
)

// Flags
var (
	randomSeed           = flag.Int64("random_seed", 0, "The random seed to use for RNG. To use a specific seed, set this flag to a non-zero value.")
	inputFile            = flag.String("input_file", "biomes.c", "The input file containing the biome data")
	outputFile           = flag.String("output_file", "simulation.png", "The output .png file to write the simulated game map to. If this flag is empty, a text version of the simulated game map is written to stdout.")
	simulatedBiomesCount = flag.Int("simulated_biomes_count", 20, "How many biomes to simulate")
)

// Constants - These should match the constants in generate_biomes.go and common.h.
const (
	pixelsPerTile   = 8
	columnsPerBiome = 20
	rowsPerColumn   = 17
	minCaveWidth    = 3
	// The below probabilities are out of 65,535 (uint16 max value).
	mineProbability         = 2000
	healthPickupProbability = 100
	shieldPickupProbability = 100
)

var rng *rand.Rand

//
// Simulation
//

type TileType int

const (
	EmptyTile TileType = iota
	BlockTile
	MineTile
	HealthTile
	ShieldTile
)

type GameMap struct {
	Tiles        [][]TileType
	ChosenBiomes []int
}

type ColumnData struct {
	TopRow uint8
	Width  uint8
}

func (gm *GameMap) fillColumn(column int, data ColumnData) {
	gm.Tiles[column] = make([]TileType, rowsPerColumn)
	for i := range gm.Tiles[column] {
		if i < int(data.TopRow) || i >= int(data.TopRow+data.Width) {
			gm.Tiles[column][i] = BlockTile
			continue
		}
		// Note: The below should match the mine and pickup generation code in procedural_generation.c.
		n := rng.Intn(math.MaxUint16 + 1)
		if n < math.MaxUint16-(mineProbability+shieldPickupProbability+healthPickupProbability) {
			gm.Tiles[column][i] = EmptyTile
		} else if n < math.MaxUint16-(shieldPickupProbability+healthPickupProbability) {
			gm.Tiles[column][i] = MineTile
		} else if n < math.MaxUint16-healthPickupProbability {
			gm.Tiles[column][i] = ShieldTile
		} else {
			gm.Tiles[column][i] = HealthTile
		}
	}
}

func (gm *GameMap) String() string {
	var sb strings.Builder
	for row := 0; row < rowsPerColumn; row++ {
		for col := 0; col < len(gm.Tiles); col++ {
			if gm.Tiles[col][row] == BlockTile {
				sb.WriteRune('X')
			} else {
				sb.WriteRune(' ')
			}
		}
		sb.WriteRune('\n')
	}
	return sb.String()
}

func GenerateGameMap(biomes [][]ColumnData, connections [][]int, simulationCount int) *GameMap {
	gameMap := &GameMap{
		Tiles:        make([][]TileType, simulationCount*columnsPerBiome),
		ChosenBiomes: make([]int, 0, simulationCount),
	}
	startIdx := 0
	biome := rng.Intn(len(biomes))
	for i := 0; i < simulationCount; i++ {
		gameMap.ChosenBiomes = append(gameMap.ChosenBiomes, biome)
		for col := 0; col < columnsPerBiome; col++ {
			gameMap.fillColumn(startIdx+col, biomes[biome][col])
		}
		startIdx += columnsPerBiome
		conn := rng.Intn(len(connections[biome]))
		biome = connections[biome][conn]
	}
	return gameMap
}

//
// I/O
//

// Unpacks the data for a column and returns the top row and width of the cave.
func unpackColumnData(data uint8) ColumnData {
	var topRow uint8 = data >> 4
	var width uint8 = data & 0x0F
	return ColumnData{
		TopRow: topRow,
		Width:  width + minCaveWidth,
	}
}

func parseBiomeData(lines []string) [][]ColumnData {
	regex := regexp.MustCompile(`\{(.*)\}`)
	biomes := [][]ColumnData{}
	for _, line := range lines {
		submatches := regex.FindStringSubmatch(line)
		if len(submatches) != 2 {
			continue
		}
		line = submatches[1]
		numStrs := strings.Split(line, ",")
		biomes = append(biomes, []ColumnData{})
		for _, numStr := range numStrs {
			numStr = strings.TrimSpace(numStr)
			if len(numStr) == 0 {
				continue
			}
			num, err := strconv.ParseUint(numStr, 0, 8)
			if err != nil {
				log.Fatalf("Failed to convert string to integer: %v", err)
			}
			data := unpackColumnData(uint8(num))
			biomes[len(biomes)-1] = append(biomes[len(biomes)-1], data)
		}
	}
	return biomes
}

func findBiomeData(lines []string) [][]ColumnData {
	regex := regexp.MustCompile(`biome_columns\[.*\].*=`)
	for i, line := range lines {
		if !regex.MatchString(line) {
			continue
		}
		for j := i + 1; j < len(lines); j++ {
			if strings.Contains(lines[j], ";") {
				return parseBiomeData(lines[i+1 : j])
			}
		}
	}
	log.Fatalf("Failed to find biome column data in input file")
	return nil
}

func parseBiomeConnections(lines []string) [][]int {
	regex := regexp.MustCompile(`\{(.*)\}`)
	connections := [][]int{}
	for _, line := range lines {
		submatches := regex.FindStringSubmatch(line)
		if len(submatches) != 2 {
			continue
		}
		line = submatches[1]
		numStrs := strings.Split(line, ",")
		connections = append(connections, []int{})
		for _, numStr := range numStrs {
			numStr = strings.TrimSpace(numStr)
			if len(numStr) == 0 {
				continue
			}
			num, err := strconv.ParseUint(numStr, 10, 8)
			if err != nil {
				log.Fatalf("Failed to convert string to integer: %v", err)
			}
			connections[len(connections)-1] = append(connections[len(connections)-1], int(num))
		}
	}
	return connections
}

func findBiomeConnections(lines []string) [][]int {
	regex := regexp.MustCompile(`next_possible_biomes\[.*\].*=`)
	for i, line := range lines {
		if !regex.MatchString(line) {
			continue
		}
		for j := i + 1; j < len(lines); j++ {
			if strings.Contains(lines[j], ";") {
				return parseBiomeConnections(lines[i+1 : j])
			}
		}
	}
	log.Fatalf("Failed to find biome connection data in input file")
	return nil
}

func printGameMap(gameMap *GameMap) {
	fmt.Print(gameMap.String())
}

// Draws a line on the given image.
func drawLine(img *image.RGBA, c color.Color, x1, y1, x2, y2 int) {
	img.Set(x1, y1, c)
	img.Set(x2, y2, c)

	dx := x2 - x1
	if dx < 0 {
		dx *= -1
	}
	dy := y2 - y1
	if dy < 0 {
		dy *= -1
	}

	steps := max(dx, dy)
	if steps > 0 {
		incX := float64(dx) / float64(steps)
		incY := float64(dy) / float64(steps)
		currX := float64(x1)
		currY := float64(y1)
		for i := 0; i <= steps; i++ {
			img.Set(int(currX), int(currY), c)
			currX += incX
			currY += incY
		}
	}
}

func fillCell(img *image.RGBA, c color.Color, row, column int) {
	startX := column * pixelsPerTile
	startY := row * pixelsPerTile
	for x := 0; x < pixelsPerTile; x++ {
		for y := 0; y < pixelsPerTile; y++ {
			img.Set(startX+x, startY+y, c)
		}
	}
}

func drawBiomeIndex(img *image.RGBA, column, index int) {
	startX := column * pixelsPerTile
	lightBlue := color.RGBA{173, 216, 230, 255}
	for x := 0; x < index; x++ {
		img.Set(startX+x, 0, lightBlue)
	}
}

func drawGameMap(img *image.RGBA, gameMap *GameMap) {
	// Fill in cells.
	black := color.RGBA{0, 0, 0, 255}
	grey := color.RGBA{128, 128, 128, 255}
	raspberry := color.RGBA{227, 11, 92, 255}
	cornflowerBlue := color.RGBA{100, 149, 237, 255}
	for col := 0; col < len(gameMap.Tiles); col++ {
		for row := 0; row < len(gameMap.Tiles[col]); row++ {
			switch gameMap.Tiles[col][row] {
			case EmptyTile:
				// No fill color.
			case BlockTile:
				fillCell(img, black, row, col)
			case MineTile:
				fillCell(img, grey, row, col)
			case HealthTile:
				fillCell(img, raspberry, row, col)
			case ShieldTile:
				fillCell(img, cornflowerBlue, row, col)
			default:
				log.Fatalf("Unknown tile type: %d", gameMap.Tiles[col][row])
			}
		}
	}

	// Draw biome indexes.
	for i, biome := range gameMap.ChosenBiomes {
		drawBiomeIndex(img, i*columnsPerBiome, biome)
	}

	// Draw bottom border.
	lightGrey := color.RGBA{211, 211, 211, 255}
	for y := rowsPerColumn * pixelsPerTile; y < (rowsPerColumn+1)*pixelsPerTile; y++ {
		for x := 0; x < len(gameMap.Tiles)*pixelsPerTile; x++ {
			img.Set(x, y, lightGrey)
		}
	}
}

func outputImage(gameMap *GameMap, file string) {
	width := len(gameMap.Tiles) * pixelsPerTile
	height := (rowsPerColumn + 1) * pixelsPerTile
	img := image.NewRGBA(image.Rect(0, 0, width, height))

	// Fill the background with white.
	white := color.RGBA{255, 255, 255, 255}
	for x := 0; x < width; x++ {
		for y := 0; y < height; y++ {
			img.Set(x, y, white)
		}
	}

	// Draw game map.
	drawGameMap(img, gameMap)

	// Draw grid lines.
	lightGrey := color.RGBA{211, 211, 211, 255}
	for x := (columnsPerBiome * pixelsPerTile) - 1; x < width; x += columnsPerBiome * pixelsPerTile {
		drawLine(img, lightGrey, x, 0, x, height)
	}

	// Write .png file.
	f, err := os.Create(file)
	if err != nil {
		log.Fatalf("Failed to create .png file: %v", err)
	}
	defer f.Close()
	if err := png.Encode(f, img); err != nil {
		log.Fatalf("Failed to write .png file: %v", err)
	}
}

//
// Main
//

func main() {
	log.Print("Simulating procedural generation...")
	flag.Parse()

	seed := *randomSeed
	if seed == 0 {
		seed = time.Now().UnixNano()
	}
	rng = rand.New(rand.NewSource(seed))
	log.Printf("Using random seed: %d", seed)

	log.Printf("Reading from input file: %s", *inputFile)
	data, err := ioutil.ReadFile(*inputFile)
	if err != nil {
		log.Fatalf("Failed to read input file: %v", err)
	}

	lines := strings.Split(string(data), "\n")
	biomes := findBiomeData(lines)
	log.Printf("Number of biomes found: %d", len(biomes))
	connections := findBiomeConnections(lines)
	log.Printf("Number of connections per biome found: %d", len(connections[0]))
	gameMap := GenerateGameMap(biomes, connections, *simulatedBiomesCount)
	if *outputFile == "" {
		printGameMap(gameMap)
	} else {
		log.Printf("Writing simulated game map to file: %s", *outputFile)
		outputImage(gameMap, *outputFile)
	}
}
