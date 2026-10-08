"""Check ROM container corruption handling using only our generated diagnostic."""
import tempfile,unittest
from pathlib import Path
from make_diagnostic_rom import make_rom
from pack_roms import pack,verify

class ContainerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.folder=Path(tempfile.mkdtemp(prefix='pack-',dir='build/host-tests'))
        cls.rom=cls.folder/'diagnostic.nes';cls.rom.write_bytes(make_rom())
    def test_named_rom_roundtrip(self):
        self.assertEqual(verify(pack([self.rom],['诊断'])),['诊断'])
    def test_payload_corruption(self):
        image=bytearray(pack([self.rom]));image[5000]^=1
        with self.assertRaises(ValueError):verify(image)
    def test_duplicate_names(self):
        with self.assertRaises(ValueError):pack([self.rom,self.rom],['重复','重复'])
    def test_incorrect_name_count(self):
        with self.assertRaises(ValueError):pack([self.rom],[])
    def test_truncated_container(self):
        with self.assertRaises(ValueError):verify(pack([self.rom])[:-1])
if __name__=='__main__':unittest.main()
