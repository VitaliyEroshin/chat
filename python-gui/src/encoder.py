import numpy as np

def get_bit(var, order):
    return (var >> order) % 2

def set_bit(var, order, value):
    if (get_bit(var, order) != value):
        var ^= (1 << order)

    return var

class Object:
    content = ""
    attributes = dict()
    object_type = "unknown"

    def set_value(self, attribute, value):
        self.attributes[attribute] = value

    def get_value(self, attribute):
        if attribute not in self.attributes:
            return None

        return self.attributes[attribute]

class Encoder:
    def get_type_id(self, type_alias):
        type_id = {
            "text" : 1,
            "loginAttempt" : 2,
            "returnCode" : 3,
            "command" : 4,
        }

        if type_alias not in type_id:
            return 0
        
        return type_id[type_alias]

    def get_attribute_order(self, attribute_alias):
        attribute_order = {
            "id" : 0,
            "author" : 1,
            "timestamp" : 2,
            "thread" : 3,
            "reply" : 4,
            "prev" : 5, 
            "next" : 6,
            "returnCode" : 7
        }
        return attribute_order[attribute_alias]

    def get_encoded_attributes(self, obj):
        attributes_encoded = 0
        for key in obj.attributes:
            attributes_encoded = set_bit(
                attributes_encoded, self.get_attribute_order(key), 1
            )
        b = attributes_encoded.to_bytes(1, "big")

        if "id" in obj.attributes:
            b += obj.attributes["id"].to_bytes(4, "big")

        if "author" in obj.attributes:
            b += obj.attributes["author"].to_bytes(4, "big")

        if "timestamp" in obj.attributes:
            b += obj.attributes["timestamp"].to_bytes(8, "big")

        if "thread" in obj.attributes:
            b += obj.attributes["thread"].to_bytes(4, "big")

        if "reply" in obj.attributes:
            b += obj.attributes["reply"].to_bytes(4, "big")

        if "prev" in obj.attributes:
            b += obj.attributes["prev"].to_bytes(4, "big")
        
        if "next" in obj.attributes:
            b += obj.attributes["next"].to_bytes(4, "big")

        if "returnCode" in obj.attributes:
            b += obj.attributes["returnCode"].to_bytes(4, "big")

        return b

    def get_encoded_type(self, obj):
        type_id = self.get_type_id(obj.object_type)
        return type_id.to_bytes(1, "big")

    def encode(self, obj):
        message = bytes()
        message += self.get_encoded_type(obj)
        message += self.get_encoded_attributes(obj)
        message += bytes(obj.content, "UTF-8")
        return message


    def get_type_name(self, type_id):
        type_names = {
            1 : "text",
            2 : "loginAttempt",
            3 : "returnCode",
            4 : "command"
        }
        if type_id not in type_names:
            return "unknown"
        
        return type_names[type_id]

    def decode(self, s):
        obj = Object()
        obj.object_type = self.get_type_name(int.from_bytes(s[0:1], "big"))
        attributes_encoded = int.from_bytes(s[1:2], "big")
        
        ptr = 2
        if get_bit(attributes_encoded, 0):
            obj.attributes["id"] = int.from_bytes(s[ptr : (ptr + 4)], "big")
            ptr += 4

        if get_bit(attributes_encoded, 1):
            obj.attributes["author"] = int.from_bytes(s[ptr : (ptr + 4)], "big")
            ptr += 4

        if get_bit(attributes_encoded, 2):
            obj.attributes["timestamp"] = int.from_bytes(s[ptr : (ptr + 8)], "big")
            ptr += 8

        if get_bit(attributes_encoded, 3):
            obj.attributes["thread"] = int.from_bytes(s[ptr : (ptr + 4)], "big")
            ptr += 4

        if get_bit(attributes_encoded, 4):
            obj.attributes["reply"] = int.from_bytes(s[ptr : (ptr + 4)], "big")
            ptr += 4

        if get_bit(attributes_encoded, 5):
            obj.attributes["prev"] = int.from_bytes(s[ptr : (ptr + 4)], "big")
            ptr += 4

        if get_bit(attributes_encoded, 6):
            obj.attributes["next"] = int.from_bytes(s[ptr : (ptr + 4)], "big")
            ptr += 4

        if get_bit(attributes_encoded, 7):
            obj.attributes["returnCode"] = int.from_bytes(s[ptr : (ptr + 4)], "big")
            ptr += 4
        
        obj.content = s[ptr: ].decode("UTF-8")
        return obj