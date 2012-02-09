require 'test/unit'
$LOAD_PATH.unshift "../lib"
$LOAD_PATH.unshift "../ext"
require 'MessagePack'

class TestMessagePack < Test::Unit::TestCase
  def test_dump_and_load
    dump_and_load_test(1)
    dump_and_load_test(100000)
    dump_and_load_test(2.43333)
    dump_and_load_test("A string")
    dump_and_load_test([1,2,3])
    dump_and_load_test({1=>2, 3 => "hallo", "test" => 4})
    dump_and_load_test([1, 1.333, "string", [333, 444, 555], {1 => 4, "hallo" => 4}])
  end

  def dump_and_load_test(obj)
    assert_equal obj, MessagePack.load(MessagePack.dump(obj))
  end
end
