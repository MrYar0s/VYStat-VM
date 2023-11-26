import yaml

INSTR_SIZES = {"Byte": 1, "HWord": 2, "Word": 4, "DWord": 8}
INSTR_BIT_SIZES = {k: v*8 for k, v in INSTR_SIZES.items()}

def name_to_snake(name: str) -> str :
    return name.replace('.', '_')

def name_to_camel(name: str) -> str :
    return "".join(x.capitalize() for x in name.lower().replace('.', '_').split("_"))

class InstrField :
    def __init__(self, descr: dict | list) :
        simple_descr = (type(descr) == list)

        (self.lo, self.hi) = descr if simple_descr else descr["bits"]
        self.is_signed = False if simple_descr else descr.get("is_signed", False)

    def get_bit_size(self) -> int :
        return 1 + self.hi - self.lo

class Instr :
    def __init__(self, name: str, descr: dict) :
        self.name = name
        self.opcode = descr["opcode"]
        self.size = descr.get("size", "DWord")
        self.is_jump = descr.get("is_jump", False)
        self.gen_parser = descr.get("gen_parser", True)
        self.gen_handler = descr.get("gen_handler", True)

        self.fields = {k: InstrField(v) for k, v in descr.get("fields", {}).items()}

    def get_sneak_name(self) -> str :
        return name_to_snake(self.name)

    def get_camel_name(self) -> str :
        return name_to_camel(self.name)

    def get_opcode_name(self) -> str :
        return "InstrOpcode::" + self.get_sneak_name()


def load_instrs(in_name: str) -> list :
    with open(in_name, 'r') as file :
        instrs = yaml.safe_load(file)

    return [Instr(instr_name, instr_descr) for instr_name, instr_descr in instrs.items()]
