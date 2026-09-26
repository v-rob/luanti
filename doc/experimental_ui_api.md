'core' namespace reference
==========================

Flex encoding format
--------------------

The Flex serialization format is a structured binary encoding designed with the
express intent of supporting painless forward and backward compatibility when
serializing complex and nested data, without sacrificing compactness like text
or hybrid binary-and-text encodings such as JSON and BSON do.

Flex encoding and decoding is provided by the `core.encode_flex()` and
`core.decode_flex()` functions, described below.

Flex uses tag bits and bytes to indicate the type of each value in the binary
stream, which allows older code to skip over newer values it doesn't recognize.
There are four basic Flex data types: numbers, strings, keys, and sequences,
the latter two of which are used to encode all aggregate and object data types.

### Format string

Flex format strings are a sequence of format specifiers that describe how to
encode or decode values. Valid format specifiers are as follows:

* `i`: A signed arbitrary-precision integer.
* `u`: An unsigned arbitrary-precision integer.
* `s`: A length-prefixed string of arbitrary length and content.
* `k`: A key value between 1 and 126 for separating object fields.
* `[...]`: Encodes a sequence delimited by start and end tags, where `...`
  contains a nested sequence of format specifiers.
* `+`: Encodes zero or more values until the next key. Must be followed by a
  `k`, `]`, or `!` format specifier.
* `*`: Encodes zero or more values until the next end of sequence tag. Must be
  followed by a `]` or `!` format specifier.
* `!`: Encodes a sequence end tag followed by any amount of raw data, used for
  Flex-encoded data that is embedded in some other data stream.
* `{...}n`: A table of `n` copies of the `...` nested format specifiers, where
  `n` may be any non-negative integer value.
* `{...}+`: Encodes a table of repeating copies of the `...` nested format
  string until the next key. Must be followed by a `k`, `]`, or `!` format
  specifier, and additionally `...` may not contain a `k` format specifier.
* `{...}*`: Encodes a table of repeating copies of the `...` nested format
  string until the next end of sequence tag. Must be followed by a `]` or `!`
  format specifier.
* ` `: Spaces are ignored.

Each format specifier takes one additional argument from `core.encode_flex()`
or returns one additional value from `core.decode_flex()` in order of their
appearance in the format string, save `[...]` which takes or returns exactly as
many values as appear in the nested `...` format string. For example:

    local data = core.encode_flex("s {i}2 [isi]", "blah", {1, 2}, 1, "two", 3)
    local s, i1, i2, contents = core.decode_flex("sii [*]", data)

If more arguments than necessary are passed to `core.encode_flex()`, the extra
arguments will be ignored. Similarly, specifiers that take tables as input will
ignore any excess values in the table.

When encoding data, the argument for `!` is optional and defaults to an empty
string. Hence, these three lines of code will yield equivalent results:

    core.encode_flex("ii!", 3, 5, "this is some raw data")
    core.encode_flex("ii!", 3, 5, "") .. "this is some raw data"
    core.encode_flex("ii!", 3, 5) .. "this is some raw data"

When decoding, the `!` specifier returns the raw data following the end of
sequence tag. This line of code will decode the two Flex-encoded integers above
as well as the raw data following them:

    local i3, i5, raw_data = core.encode_flex("ii!", data)
    assert(raw_data == "this is some raw data")

The `+` and `*` specifiers take and return Flex-encoded data strings. When
encoding, the data string is not inserted verbatim as this could lead to
malformed data, as in the following case where an extraneous sequence end tag
would be inserted by the `+` before the actual end of the sequence:

    local data = core.encode_flex("i!", 2, "appended data")
    core.encode_flex("[+ki]", data, 17, 5)

To prevent this sort of problem, the data string is decoded according to the
same format specifier before being encoded. That is, the second line above will
actually be interpreted like

    core.encode_flex("[+ki]", core.decode_flex("+", data), 17, 5)

which trims off the extraneous sequence end tag and following data.

TODO: Describe table specifiers

### Forwards compatibility

TODO: Document reading wrong type of data and stuff
TODO: Also document how end of stream is treated

### Integer encoding

Flex integers use a variable-length encoding, which permits arbitrary-precision
integers that can be interpreted as either signed or unsigned based on how the
consumer reads them. For small integer values, this results in a much more
compact encoding, while imposing very little overhead for large integer values.

The upper two bits in the first byte contain a tag of `0b10` to indicate that
this is a number. The lowest bit in each byte indicates whether there are more
bytes in the sequence, with a zero bit ending the sequence. The remaining bits
in each byte encode the number in big-endian order.

As an example, here is the Flex encoding of the unsigned integer 107014, or
`0b110'1000100'0000110` in binary:

    Tag in first byte to indicate that this is an integer
    |    Upper five bits of the integer
    |    |   Continuation bit
    |    |   |
    |    |   |     Middle seven bits of the integer
    |    |   |     |    Continuation bit
    |    |   |     |    |
    |    |   |     |    |     Lower seven bits of the integer
    |    |   |     |    |     |    Termination bit
    |    |   |     |    |     |    |
    vv vvvvv v  vvvvvvv v  vvvvvvv v
    10'00110'1  1000100'1  0000110'0

Note that the integer requires only 17 bits, so the encoded unsigned integer
must be zero-extended to 5 + 7 + 7 = 19 bits. If it were a signed integer, the
value would have to be sign-extended instead. Nothing prevents the same integer
from being encoded with more than three bytes, but the shortest encoding is
naturally preferred by `core.encode_flex()`.

TODO: precision limitations, floats

### String encoding

Strings are encoded as a length-prefixed sequence of bytes, allowing arbitrary
binary data to be contained in the string. The length prefix is encoded the
same as an unsigned integer, but with a tag of `0b11` instead of `0b10` to
indicate that this is a string.

For example, the 13-byte string "Hello, world!" is encoded in Flex like so:

    Tag in first byte to indicate that this is a string
    |    Five bits encoding the length of the string
    |    |   Termination bit
    |    |   |                           Contents of the string
    |    |   |                           |
    vv vvvvv v   v---v---v---v---v---v---v---v---v---v---v---v---v
    11'01101'0  'H' 'e' 'l' 'l' 'o' ',' ' ' 'w' 'o' 'r' 'l' 'd' '!'

### Sequence encoding

Sequences are used to represent aggregate data structures. Sequences start with
a tag byte of `0x7F`, followed by any number of Flex values (which may include
nested sequences), and terminate with another tag byte of `0x00`.

For instance, an array could be represented as a sequence of integers. Or, an
associative array could be encoded as a sequence with alternating string keys
and values. As an example of the latter, the table `{key = 5, example = 31}`
could be encoded as follows:

        Sequence begin tag
        |                 First key "key"
        |                 |                  First value 5
        |                 |                  |
    v-vvvvvvv  vv-vvvvv-v---v---v---v   vv-vvvvv-v
    0'1111111  11'00011'0  'k' 'e' 'y'  10'00101'0   >--v
                                                        |
     v--------------------------------------------------<
     |
     >-->   11'00111'0  'e' 'x' 'a' 'm' 'p' 'l' 'e'  10'11111'0  0'0000000
            ^^-^^^^^-^---^---^---^---^---^---^---^   ^^-^^^^^-^  ^-^^^^^^^
                               |                          |          |
                               |                          |          Sequence end tag
                               |                          Second value 31
                               Second key "example"

TODO: Describe objects

### Key sequence encoding

Sequences can also be used to represent objects without restorting to a
string-encoded associative array by using key tags, which are single byte
tags that range from `0x01` to `0x7E`. These can be used to indicate the
start of a field in an object, followed by zero or more values that define
the value of the field.

Suppose for instance that a struct has the following fields:

  - `name`, a string assigned to key `0x01`
  - `id`, an integer assigned to key `0x02`
  - `position`, a pair of integers assigned to key `0x03`

Then to encode the struct `{.name = "boxy", .position = {22, -3}}` with the
`id` field intentionally omitted, this struct could be encoded as follows:

        Sequence begin tag
        |          Tag `0x01` indicating start of `name` field
        |          |                   String "boxy"
        |          |                   |
    v-vvvvvvv  v-vvvvvvv  vv-vvvvv-v---v---v---v---v
    0'1111111  0'0000001  11'00100'0  'b' 'o' 'x' 'y'   >--v
                                                           |
     v-----------------------------------------------------<
     |
     >-->   0'0000011  10'00000'1  0010110'0  10'11101'0  0'0000000
            ^-^^^^^^^  ^^-^^^^^-^--^^^^^^^-^  ^^-^^^^^-^  ^-^^^^^^^
                |                |                 |          |
                |                |                 |          Sequence end tag
                |                |                 Y coordinate -3 of position
                |                X coordinate 22 of position
                Tag `0x03` indicating start of `position` field

There are a few notable advantages to using keys to structure objects.
First, the fields of the object do not have to be in any particular order,
and optional fields can be omitted to decrease the size of the encoded data.

Moreover, it becomes simple to expand the protocol without breaking
backward compatibility. If a `visible` field were added to the struct and
assigned to key `0x04`, the new key would be ignored by older code when
decoding the object. Similarly, if the `position` field were expanded to
have a third Z component, older code would read the first two values and
ignore the third, passing on to the next key in the sequence.

Without keys, it is possible to solve these problems by appending all new
fields to the end of the sequence, but this would result in an increasingly
large number of required values in the object. Keys make the encoding much
more flexible with very little extra overhead.

It is also possible to use a hybrid model to decrease overhead even further:
required fields can be placed at the start of the sequence before any keys,
followed by zero or more keys containing optional data.

Note that sequence begin tags, end tags, and keys all have an upper bit of
zero, which distinguishes them from integer and string tags.

TODO: Describe how backwards compatibility works

### Embedded Flex data

Flex-encoded data is very amenable to being embedded inside other data, such as
ASCII or UTF-8 text. An ASCII control character such as ESC `0x27` or similar
can be used to indicate the start of the Flex-encoded data, followed by zero or
more Flex values, terminated by a sequence end tag `0x00`. This allows ASCII
text and properly delimited Flex-encoded data to be concatenated at will:

                     Regular ASCII text
                     |                       ESC character denoting start of Flex data
                     |                       |
     v---v---v---v---v---v---v---v---v   v-vvvvvvv
    'S' 'o' 'm' 'e' ' ' 't' 'e' 'x' 't'  0'0011011   >--v
                                                       |
     v-------------------------------------------------<
     |
     >-->   10'01010'1  0110001'0  0'0000000  'm' 'o' 'r' 'e' ' ' 't' 'e' 'x' 't'
            ^^-^^^^^-^--^^^^^^^-^  ^-^^^^^^^   ^---^---^---^---^---^---^---^---^
                      |                |                       |
                      |                |                       More regular ASCII text
                      |                Sequence end tag denoting end of Flex data
                      Arbitrary Flex-encoded data

TODO: Fix this description

This technique works because end-of-stream conditions and sequence end tags
are treated identically by the `core.decode_flex()`. To decode this sort of
data, a `FlexDecoder` can be instantiated on the entire string following the
ASCII ESC character. The Flex data can be read as normal, and after reading the
sequence end tag, the rest of the string can be interpreted as ASCII again.

### Functions

* `core.encode_flex(fmt, ...) -> data`: Returns a Flex-encoded binary `data`
  string containing the arguments encoded according to the format string `fmt`.
* `core.decode_flex(fmt, data) -> ...`: Returns the values decoded from the
  Flex-encoded binary `data` string according to the format string `fmt`.
