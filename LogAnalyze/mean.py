#!/usr/bin/env python
# -*- coding: utf-8 -*-

class OnlineMean:
	def __init__(self):
		self.value = 0
		self.size = 0
	
	def add_value(self, x):
		self.size += 1
		self.value += (x - self.value) / float(self.size);
		return self.value

if __name__ == '__main__':
	mean = OnlineMean()
	mean.add_value(20)
	mean.add_value(30)
	mean.add_value(40)
	print(mean.value)