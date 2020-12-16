require 'socket'

BaseDir = File.dirname(File.dirname(File.expand_path(__FILE__)))

class Hive
  def initialize(n)
    @n = n
    @who = {}
  end
  def start!
    system "#{BaseDir}/start.sh #{@n}"
    wait
  end
  def stop!
    system "#{BaseDir}/stop.sh all"
    sleep 0.1
  end
  def wait
    loop do
      _who
      break if @who.values.select {|m| %w(L F).include? m['s']}.size == @n
      sleep 0.1
    end
  end
  def who
    wait
    @who.values.map {|m| m['f']}.first
  end
  def proxy(cmd, i=1)
    IO.popen("redis-cli -p #{7000 + i} #{cmd}") do |cli|
      cli.readlines
    end
  end
  def _who
    @who.clear
    (1..@n).each do |i|
      begin
        s = TCPSocket.new 'localhost', 3000 + i
        s.write 'e=who'
        data = s.read
        s.close

        info = {}
        data.split('|').each do |part|
          a,b = part.split('=')
          info[a] = b
        end
        @who[info['a']] = info
      rescue
      end
    end
    @who
  end
end

