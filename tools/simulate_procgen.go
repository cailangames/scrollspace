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

type tileType int

const (
	emptyTile tileType = iota
	blockTile
	mineTile
	healthTile
	shieldTile
)

type columnData struct {
	topRow uint8
	width  uint8
}

// Unpacks the data for a column and returns the top row and width of the cave.
func unpackColumnData(data uint8) columnData {
	var topRow uint8 = data >> 4
	var width uint8 = data & 0x0F
	return columnData{
		topRow: topRow,
		width:  width + minCaveWidth,
	}
}

func parseBiomeData(lines []string) [][]columnData {
	biomes := [][]columnData{}
	for _, line := range lines {
		line = strings.TrimSpace(line)
		line = strings.ReplaceAll(line, "{", "")
		line = strings.ReplaceAll(line, "},", "")
		numStrs := strings.Split(line, ",")
		if len(numStrs) <= 1 {
			continue
		}
		biomes = append(biomes, []columnData{})
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

func findBiomeData(lines []string) [][]columnData {
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
	connections := [][]int{}
	for _, line := range lines {
		line = strings.TrimSpace(line)
		line = strings.ReplaceAll(line, "{", "")
		line = strings.ReplaceAll(line, "},", "")
		numStrs := strings.Split(line, ",")
		if len(numStrs) <= 1 {
			continue
		}
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

func fillColumn(gameMap [][]tileType, column int, data columnData) {
	gameMap[column] = make([]tileType, rowsPerColumn)
	for i := range gameMap[column] {
		if i < int(data.topRow) || i >= int(data.topRow+data.width) {
			gameMap[column][i] = blockTile
			continue
		}
		// Note: The below should match the mine and pickup generation code in procedural_generation.c.
		n := rng.Intn(math.MaxUint16 + 1)
		if n < math.MaxUint16-(mineProbability+shieldPickupProbability+healthPickupProbability) {
			gameMap[column][i] = emptyTile
		} else if n < math.MaxUint16-(shieldPickupProbability+healthPickupProbability) {
			gameMap[column][i] = mineTile
		} else if n < math.MaxUint16-healthPickupProbability {
			gameMap[column][i] = shieldTile
		} else {
			gameMap[column][i] = healthTile
		}
	}
}

func generateGameMap(biomes [][]columnData, connections [][]int, simulationCount int) ([][]tileType, []int) {
	gameMap := make([][]tileType, simulationCount*columnsPerBiome)
	chosenBiomes := make([]int, 0, simulationCount)
	startIdx := 0
	biome := rng.Intn(len(biomes))
	for i := 0; i < simulationCount; i++ {
		chosenBiomes = append(chosenBiomes, biome)
		for col := 0; col < columnsPerBiome; col++ {
			fillColumn(gameMap, startIdx+col, biomes[biome][col])
		}
		startIdx += columnsPerBiome
		conn := rng.Intn(len(connections[biome]))
		biome = connections[biome][conn]
	}
	return gameMap, chosenBiomes
}

func printGameMap(gameMap [][]tileType) {
	for row := 0; row < rowsPerColumn; row++ {
		for col := 0; col < len(gameMap); col++ {
			var s string
			if gameMap[col][row] == blockTile {
				s = "X"
			} else {
				s = " "
			}
			fmt.Print(s)
		}
		fmt.Println()
	}
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

func outputImage(gameMap [][]tileType, chosenBiomes []int, file string) {
	width := len(gameMap) * pixelsPerTile
	height := rowsPerColumn * pixelsPerTile
	img := image.NewRGBA(image.Rect(0, 0, width, height))

	// Fill the background with white.
	white := color.RGBA{255, 255, 255, 255}
	for x := 0; x < width; x++ {
		for y := 0; y < height; y++ {
			img.Set(x, y, white)
		}
	}

	// Fill in cells.
	black := color.RGBA{0, 0, 0, 255}
	grey := color.RGBA{128, 128, 128, 255}
	raspberry := color.RGBA{227, 11, 92, 255}
	cornflowerBlue := color.RGBA{100, 149, 237, 255}
	for col := 0; col < len(gameMap); col++ {
		for row := 0; row < len(gameMap[col]); row++ {
			switch gameMap[col][row] {
			case emptyTile:
				// No fill color.
			case blockTile:
				fillCell(img, black, row, col)
			case mineTile:
				fillCell(img, grey, row, col)
			case healthTile:
				fillCell(img, raspberry, row, col)
			case shieldTile:
				fillCell(img, cornflowerBlue, row, col)
			default:
				log.Fatalf("Unknown tile type: %d", gameMap[col][row])
			}
		}
	}

	// Draw biome indexes.
	for i := 0; i < len(chosenBiomes); i++ {
		drawBiomeIndex(img, i*columnsPerBiome, chosenBiomes[i])
	}

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
	gameMap, chosenBiomes := generateGameMap(biomes, connections, *simulatedBiomesCount)
	if *outputFile == "" {
		printGameMap(gameMap)
	} else {
		log.Printf("Writing simulated game map to file: %s", *outputFile)
		outputImage(gameMap, chosenBiomes, *outputFile)
	}
}
