import unittest, vivarium
class TestVivarium(unittest.TestCase):
 def test_seed(self):
  a=vivarium.make("Navigate-v0",seed=4);b=vivarium.make("Navigate-v0",seed=4);self.assertEqual(a.reset(seed=4)[0],b.reset(seed=4)[0])
 def test_five_tuple(self):
  e=vivarium.make("PoleBalance-v0",seed=2);r=e.step([0]);self.assertEqual(len(r),5);self.assertEqual(len(r[0]),4)
 def test_tasks(self):
  for name,act in [("BallPush-v0",[0,0]),("Navigate-v0",[0,0]),("PoleBalance-v0",[0]),("StackBlocks-v0",[0])]:
   e=vivarium.make(name,seed=1);self.assertTrue(e.reset()[0]);self.assertEqual(len(e.step(act)),5)
if __name__=="__main__":unittest.main()
