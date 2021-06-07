import os

from ctypes import CDLL, c_char, c_char_p, c_int, c_void_p, Structure, POINTER

class ConversionResult(Structure):
    _fields_ = [("size", c_int), ("buffer", c_void_p)]

def encode_and_print_as_xml(enc_functions, message_id, protocol_version, message_filename):
    encoding_function = enc_functions.encodeToXer
    encoding_function.argtypes = (c_int, c_int, c_char_p, c_int)
    encoding_function.restype = POINTER(ConversionResult)

    file_size = os.path.getsize(message_filename)

    with open(message_filename, "rb") as message:
        data = bytearray(file_size)
        written = message.readinto(data)
    
    data = data[:written]
    char_array = c_char * written
    data_array = char_array.from_buffer(data)

    encoding_result = encoding_function(message_id, protocol_version, data_array, written)

    if encoding_result.contents.size < 0:
        print("Failed to encode!")
        enc_functions.freeConversionResult(encoding_result)
        return
    
    print("Encoded successfully!")
    xml_message = c_char_p(encoding_result.contents.buffer)
    # Last symbol will be a new line, print() already adds a \n
    print(xml_message.value.decode()[:-1])
    
    enc_functions.freeConversionResult(encoding_result)

def main():
    so_filename = "./converter-so/converter"
    my_functions = CDLL(so_filename)

    message_id = 5
    protocol_version = 2
    # message_filename = "/mnt/e/Paul/Desktop/spatem_siemens.uper.bin"
    message_filename = "/mnt/e/Paul/Desktop/mapem_kurt.uper.bin"

    encode_and_print_as_xml(my_functions, message_id, protocol_version, message_filename)

main()
