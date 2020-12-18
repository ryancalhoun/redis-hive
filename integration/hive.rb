require 'redis'
require 'socket'
require 'timeout'

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
  def stop(node: [], redis: [])
    system "#{BaseDir}/stop.sh #{node.map {|n| "-v#{n}"}.join(' ')} #{redis.map {|n| "-r#{n}"}.join(' ')}"
    sleep 0.1
  end
  def wait
    t0 = Time.now

    loop do
      _who

      t1 = Time.now
      if t1 - t0 > 10
        raise Timeout::Error.new("Expired after #{t1-t0} seconds. Nodes: #{
            @who.values.map {|m| "#{m['a']}/#{m['s']}"}.join(", ")}. Redis replicas: #{
            redis_info['connected_slaves']}.")
      end

      break if
        @who.values.select {|m| %w(L F).include? m['s']}.size == @n &&
        redis_info['connected_slaves'].to_i == @n - 1

      sleep 0.1
    end
  end
  def who
    wait
    @who
  end
  def leader
    wait
    @who.values.map {|m| m['f']}.first
  end
  def _redis(cmd, *args, port:)
    redis = Redis.new port: port
    return redis.method(cmd).call *args
  ensure
    redis.close
  end
  def proxy_redis_command(cmd, *args, i: 1)
    _redis cmd, *args, port: 7000 + i
  end
  def direct_redis_command(cmd, *args, i: 1)
    _redis cmd, *args, port: 6000 + i
  end
  def redis_info(i=1)
    proxy_redis_command('info', i: i)
  end
  def _who
    @who.clear
    (1..@n).each do |i|
      begin
        info = direct_who(i)
        @who[info['a']] = info
      rescue
      end
    end
    @who
  end
  def direct_who(i)
    s = TCPSocket.new 'localhost', 3000 + i
    s.write 'e=who'
    data = s.read
    s.close

    info = {}
    data.split('|').each do |part|
      a,b = part.split('=')
      info[a] = b
    end
    info
  end
end

