#!/usr/bin/python

from subprocess import PIPE, Popen
from optparse import OptionParser

#
class Node:
	def __init__(self, name):
		self.name = name
   
	def execute(self):
		self.stream = Popen (self.name + args, stdout=PIPE)
 
#
class Launcher:
	def __init__(self):
		self.parser = OptionParser()
		self.nqueries = 0
		self.parser.add_option ("-n", "--nqueries", dest=self.nqueries,
                            help="Number of back-end servers")

	def setup(self):
		for i in range (self.nqueries):
			self.backend.append (Node("raven01"))

	def execute(self):
		for i in range (self.nqueries):
			self.backend[i].execute()


launcher = Launcher ()
launcher.setup()
launcher.execute()


