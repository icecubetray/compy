I love, among others, sushi, pizza, and pasta.


## File format

```
magic: 4 bytes
map_count: 1 byte
map_node: 3-? bytes
data: * bytes

magic
0x20 0x16 0x11 0x27

map_count
(0x00 - 0xFF)

map_node
value: 1 byte, (0x00 - 0xFF)
bit_count: 1 byte, (0x00 - 0xFF)
data: ((bit_count / 8) + 1) bytes, (0x00 - 0xFF)*

data
(0x00 - 0xFF)*
```
