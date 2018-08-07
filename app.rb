dio = DigitalIO.new
v = 0

loop {
  10.times {|i|
    puts 'Hello, ' * i + 'mruby!'
  }

  [1, 21.0975, 'mruby', [1, 4, 7]].each {|v|
    puts (v * 2).to_s
  }

  8.times {
    v ^= 1
    dio.write v
    delay(250)
  }

  GC.start
}
