package main

import (
	"fmt"
	"image"
	"image/color"
	"image/png"
	"io/ioutil"
	"log"
	"os"
	"regexp"
	"strconv"
	"strings"
)

// Parameters
const (
	// To use a specific seed for the RNG, set `randomSeed` to a non-zero value.
	randomSeed int64 = 0
	inputFile        = "../src/procedural_generation.c"
	// If `outputFile` is empty, a text version of the generated map is written to stdout.
	outputFile         = "simulation.png"
	simulateBiomeCount = 20
)

// Constants - These should match the constants in generate_biomes.go.
const (
	pixelsPerTile   = 8
	columnsPerBiome = 20
	rowsPerColumn   = 17
	minCaveWidth    = 3
)

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
	log.Fatalf("Failed to find biome column data in input file %q", inputFile)
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
	log.Fatalf("Failed to find biome connection data in input file %q", inputFile)
	return nil
}

func fillColumn(gameMap [][]tileType, column int, data columnData) {
	gameMap[column] = make([]tileType, rowsPerColumn)
	for i := range gameMap[column] {
		if i >= int(data.topRow) && i < int(data.topRow+data.width) {
			gameMap[column][i] = emptyTile
		} else {
			gameMap[column][i] = blockTile
		}
	}
}

func generateGameMap(biomes [][]columnData, connections [][]int) [][]tileType {
	gameMap := make([][]tileType, simulateBiomeCount*columnsPerBiome)
	startIdx := 0
	for biome := 0; biome < simulateBiomeCount; biome++ {
		for col := 0; col < columnsPerBiome; col++ {
			fillColumn(gameMap, startIdx+col, biomes[biome][col])
		}
		startIdx += columnsPerBiome
	}
	return gameMap
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

func outputImage(gameMap [][]tileType, file string) {
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
	for col := 0; col < len(gameMap); col++ {
		for row := 0; row < len(gameMap[col]); row++ {
			if gameMap[col][row] == blockTile {
				fillCell(img, black, row, col)
			}
		}
	}

	// Draw grid lines.
	lightGrey := color.RGBA{211, 211, 211, 255}
	for x := 0; x < width; x += columnsPerBiome * pixelsPerTile {
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

	log.Printf("Reading from input file: %s", inputFile)
	data, err := ioutil.ReadFile(inputFile)
	if err != nil {
		log.Fatalf("Failed to read input file: %v", err)
	}

	lines := strings.Split(string(data), "\n")
	biomes := findBiomeData(lines)
	log.Printf("Number of biomes found: %d", len(biomes))
	connections := findBiomeConnections(lines)
	log.Printf("Number of connections per biome found: %d", len(connections[0]))
	gameMap := generateGameMap(biomes, connections)
	if outputFile == "" {
		printGameMap(gameMap)
	} else {
		log.Printf("Writing simulated game map to file: %s", outputFile)
		outputImage(gameMap, outputFile)
	}
}
