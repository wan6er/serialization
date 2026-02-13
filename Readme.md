Basic Serialization Example
1. Define a `TestStructA` that inherit from `SERIALIZATION::Serialization`
```
struct TestStructA : public Serialization
{
    SUint32 a = { this, "1", 123 };
    SFloat32 b = { this, "2", 456.0f };
};
```
2. Instance it and then you can get JSON Object.
```
TestStructA a;
std::cout << a.get_json().dump() << std::endl;
```
3. Finally, got JSON output
```
{"1":123,"2":456.0}
```