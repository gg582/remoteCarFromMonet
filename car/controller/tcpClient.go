package main

import (
	"bufio"
	"encoding/binary"
	"encoding/json"
	"fmt"
	"io"
	"log"
	"net"
	"os"
)

type Cartype struct {
	Sr04Val uint32
	Ir0Val  uint32
	Ir1Val  uint32
}

func main() {

	upload, err := net.Listen("tcp", ":10101")

	if err != nil {
		panic(err)
	}

	uport, err := upload.Accept()
	ch := make(chan bool)
	if err != nil {
		panic(err)
	}

	go READ(uport, ch)
	for {
		if <-ch == true {
			println("send : OK")
		} else {
			println("send : failed")

		}
	}
}

func READ(uport net.Conn, ch chan bool) {
	sr04FILE, err := os.OpenFile("/dev/car/sr04", os.O_RDONLY, 0775)
	if err != nil {
		log.Println(err)
	}
	ir0FILE, err := os.OpenFile("/dev/car/ir0", os.O_RDONLY, 0775)
	if err != nil {
		log.Println(err)
	}
	ir1FILE, err := os.OpenFile("/dev/car/ir1", os.O_RDONLY, 0775)
	if err != nil {
		log.Println(err)
	}
	sr04 := bufio.NewReader(sr04FILE)
	ir0 := bufio.NewReader(ir0FILE)
	ir1 := bufio.NewReader(ir1FILE)
	for {

		var car Cartype

		byte1, err := io.ReadAll(sr04)
		if len(byte1) < 4 {
			for i := 0; i < 4; i++ {
				byte1 = append(byte1, 1)
			}
		}
		if err != nil {
			log.Println(err)
		}
		car.Sr04Val = binary.LittleEndian.Uint32(byte1)

		byte2, _ := io.ReadAll(ir0)

		if len(byte2) < 4 {
			for i := 0; i < 4; i++ {
				byte2 = append(byte2, 0)
			}
		}

		car.Ir0Val = binary.LittleEndian.Uint32(byte2)

		byte3, _ := io.ReadAll(ir1)
		if len(byte3) < 4 {
			for i := 0; i < 4; i++ {
				byte3 = append(byte3, 0)
			}
		}

		car.Ir1Val = binary.LittleEndian.Uint32(byte3)

		if car.Ir0Val > 0 {
			car.Ir0Val = 1
		}

		if car.Ir1Val > 0 {
			car.Ir1Val = 1
		}
		byte4, err := json.Marshal(&car)
		if err != nil {
			println("marshal error")
			sr04FILE.Close()
			ir0FILE.Close()
			ir1FILE.Close()
			return
		}
		byte4 = append(byte4, byte('\n'))
		fmt.Printf("sr04:%d ir0:%d ir1:%d \n", car.Sr04Val, car.Ir0Val, car.Ir1Val)
		_, err = uport.Write(byte4)
		fmt.Println(string(byte4), len(byte4))
		if err != nil {
			log.Println(err)
			ch <- false
		} else {
			fmt.Println("success")
			ch <- true
		}
	}
	sr04FILE.Close()
	ir0FILE.Close()
	ir1FILE.Close()
}
