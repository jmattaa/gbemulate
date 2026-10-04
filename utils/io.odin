package utils

import "core:fmt"
import "core:os"

fread :: proc(fname: string, nbytes: ^i64) -> []byte {
	f, err := os.open(fname)
	if err != nil {
		fmt.println(err)
		return nil
	}
	defer os.close(f)

	nbytes^, err = os.seek(f, 0, .End)
	if err != nil {
		fmt.println(err)
		return nil
	}

	if _, err := os.seek(f, 0, .Start); err != nil {
		fmt.println(err)
		return nil
	}

    buffer := make([]byte, nbytes^)
    _, err = os.read(f, buffer)
    if err != nil {
        fmt.println(err)
        return nil
    }
    return buffer
}
