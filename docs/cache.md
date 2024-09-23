# Cache

## API
Format of API is as follows
```
<version>/<Data Type>
```

To start with only data type will be supported is string map.

### JSON Parameter
```
{
    "Param1": "Value2",
    "Param2: "Value2"
    ...
}
```

### Create
```
Method: POST
Parameters: name
Response: OK or error code with Parameter as reason
```
There can be additoinal parameters based on Type.

### Delete
```
Method: DELETE
Parameters: name
```

### Add entry
```
Method: PUT
Parameters: name, key, value
```

### Retrieve entry
```
Method: GET
Parameters: name, key
Response: value
```

### Delete entry
```
Method: DELETE
Parameters: Name, Key
```
