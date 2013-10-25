require 'transform/task_runner'
require 'transform/export/transform_task'

class TestTask < TransformTask
  @@pee = 0
  def initialize(name)
    super()
    @me = name#@@pee
    @@pee += 1
    define_execution do
      puts "I am #{@me}"
    end
  end

  def describe
    @me.to_s
  end
end

class TestTaskRunning < TransformTask
  def initialize
    super
    define_execution do
      puts 'Waiting...'
      sleep 2 + rand() * 3
    end
  end
end

if ARGV[0] == 'test'
  test_root = TestTask.new('root')
  test = TestTask.new('a')
  test2 = TestTask.new('b')
  test3 = TestTask.new('c')
  run_task = TestTaskRunning.new
  test_root.depend_on(test)
  test.depend_on(test2)
  test.depend_on(test3)
  test2.depend_on(run_task)
  test3.depend_on(run_task)

  test_branch = TestTask.new('branch')
  test_branch_run = TestTaskRunning.new
  test_branch.depend_on(test_branch_run)
  test_branch.depend_on(run_task)
  test_root.depend_on(test_branch)

  TaskRunnerStackBased.new.run(test_root)
end
