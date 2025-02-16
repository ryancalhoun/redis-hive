require 'redis'
require 'socket'
require 'timeout'

BaseDir = File.dirname(File.dirname(File.expand_path(__FILE__)))

class Hive
  def initialize(n, auth: nil)
    @n = n
    @who = {}
    @auth = auth
  end
  def start!
    ENV['REDIS_AUTH'] = @auth
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
    opts = { port: port, connect_timeout: 0.5, reconnect_attempts: 0 }
    opts[:password] = @auth if @auth

    redis = Redis.new opts
    return redis.method(cmd).call *args
  rescue Redis::TimeoutError
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
    t0 = Time.now

    info = {}

    s = TCPSocket.new 'localhost', 3000 + i
    s.write 'e=who'

    begin
      data = s.read_nonblock(80)
      data.split('|').each do |part|
        a,b = part.split('=')
        info[a] = b
      end
    rescue IO::WaitReadable
      if Time.now - t0 < 1
        IO.select([s], nil, nil, 0.3)
        retry
      end
    end

    s.close
    info
  end
end

