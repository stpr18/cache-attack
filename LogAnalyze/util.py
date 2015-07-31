#!/usr/bin/env python
# -*- coding: utf-8 -*-

class Util:
	@staticmethod
	def int2q0(x):
		return x >> 24
	
	@staticmethod
	def int2q1(x):
		return (x >> 16) & 0xff
	
	@staticmethod
	def int2q2(x):
		return (x >> 8) & 0xff
	
	@staticmethod
	def int2q3(x):
		return x & 0xff
	
	@staticmethod
	def max_count(l):
		return l.count(max(l))
