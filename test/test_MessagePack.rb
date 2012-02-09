require 'test/unit'
$LOAD_PATH.unshift "../lib"
$LOAD_PATH.unshift "../ext"
require 'MessagePack'

class TestMessagePack < Test::Unit::TestCase
  OBJS = [
    1,
    100000,
    2.43333,
    "A string",
    [1,2,3],
    {1=>2, 3 => "hallo", "test" => 4},
    [1, 1.333, "string", [333, 444, 555], {1 => 4, "hallo" => 4}]
  ]

  def test_dump_and_load
    OBJS.each {|obj|
      assert_equal obj, MessagePack.load(MessagePack.dump(obj))
    }
  end

  def test_dump_and_load_file
    filename = "test.msgpack"
    OBJS.each {|obj|
      MessagePack.dump_to_file(obj, filename)
      assert_equal obj, MessagePack.load_from_file(filename)
    }
    File.delete(filename)
  end
end
