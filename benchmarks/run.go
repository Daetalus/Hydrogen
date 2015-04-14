package main

import (
	"os"
	"os/exec"
	"time"
	"fmt"
	"math"
	"strings"
	"path/filepath"
)


const (
	Iterations = 30

	Normal  = "\x1B[0m"
	Bold    = "\x1B[1m"
	Red     = "\x1B[31m"
	Green   = "\x1B[32m"
	Yellow  = "\x1B[33m"
	Blue    = "\x1B[34m"
	Magenta = "\x1B[35m"
	Cyan    = "\x1B[36m"
	White   = "\x1B[37m"
)


type Language struct {
	Name string
	Folder string
	Command []string
	Extension string
}


type Benchmark struct {
	DisplayName string
	Name string
}


func main() {

	//
	//  Languages
	//

	hydrogen := Language{
		Name: "Hydrogen",
		Folder: "hydrogen",
		Command: []string{"../build/interpreter"},
		Extension: "hy",
	}

	lua := Language{
		Name: "Lua",
		Folder: "lua",
		Command: []string{"lua5.1"},
		Extension: "lua",
	}

	luajit := Language{
		Name: "LuaJIT",
		Folder: "lua",
		Command: []string{"luajit", "-joff"},
		Extension: "lua",
	}

	python2 := Language{
		Name: "Python 2.7.8",
		Folder: "python",
		Command: []string{"python"},
		Extension: "py",
	}

	python3 := Language{
		Name: "Python 3.4.2",
		Folder: "python",
		Command: []string{"python3"},
		Extension: "py",
	}



	//
	//  Benchmarks
	//

	fib := Benchmark{
		DisplayName: "Fibonacci",
		Name: "fib",
	}



	RunBenchmarks(
		[]Language{hydrogen, lua, luajit, python2, python3},
		[]Benchmark{fib},
	)
}


func RunBenchmarks(languages []Language, benchmarks []Benchmark) {
	for _, language := range languages {
		for _, benchmark := range benchmarks {
			RunBenchmark(language, benchmark)
		}
	}
}


func RunBenchmark(language Language, benchmark Benchmark) {
	fmt.Println(Bold + White + strings.Repeat("-", 80) + Normal)
	fmt.Println(Bold + Blue + "Benchmark: " + White + benchmark.DisplayName)
	fmt.Println(Bold + Blue + " Language: " + White + language.Name)
	fmt.Print(Bold + Blue + "Run Times: " + Normal)

	path, err := filepath.Abs(filepath.Dir(os.Args[0]))
	if err != nil {
		fmt.Println(Bold + Red + "Path Error:" + White, err, Normal)
		return
	}

	average := 0.0
	times := []float64{}
	for i := 0; i < Iterations; i++ {
		args := language.Command
		args = append(args,
			language.Folder + "/" + benchmark.Name + "." + language.Extension)
		args = args[1:]
		command := exec.Command(language.Command[0], args...)
		command.Dir = path

		start := time.Now()
		err := command.Run()
		time := time.Since(start)

		if err != nil {
			fmt.Println(Bold + Red + "Error:" + White, err, Normal)
			return
		}

		seconds := time.Seconds()
		times = append(times, seconds)
		average += seconds

		fmt.Printf("%.3f ", seconds)
		if (i + 1) % 5 == 0 {
			fmt.Print("\n           ")
		}
	}

	average /= Iterations

	stddev := 0.0
	for _, time := range times {
		stddev += (average - time) * (average - time)
	}
	stddev = math.Sqrt(stddev)

	fmt.Printf(Bold + Blue + "\n           Average: " + White + "%.3f\n" + Normal, average)
	fmt.Printf(Bold + Blue + "Standard Deviation: " + White + "%.3f\n" + Normal, stddev)
}
