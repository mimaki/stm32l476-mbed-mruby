loop {
  puts 'Hello, mruby!'

  [1, 21.0975, 'mruby', [1, 4, 7]].each {|v|
    puts (v * 2).to_s
  }

  delay(1000)
}
