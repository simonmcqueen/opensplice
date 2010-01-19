namespace test.sacs
{
    public class HistDataWConditionInit : Test.Framework.TestItem
    {
        public HistDataWConditionInit()
            : base("Initialize historical data with condition"
                )
        {
        }

        public override Test.Framework.TestResult Run(Test.Framework.TestCase testCase)
        {
            DDS.DomainParticipantFactory factory;
            DDS.IDomainParticipant participant;
            DDS.DomainParticipantQos pqosHolder;
            DDS.SubscriberQos subQosHolder;
            DDS.DataReaderQos drQosHolder;
            DDS.ISubscriber subscriber;
            DDS.PublisherQos pubQosHolder;
            DDS.IPublisher publisher;
            mod.tstTypeSupport typeSupport;
            DDS.TopicQos tQosHolder;
            DDS.ITopic topic;
            DDS.DataWriterQos dwQosHolder;
            mod.tstDataWriter datawriter;
            Test.Framework.TestResult result;
            DDS.ReturnCode rc;
            result = new Test.Framework.TestResult("Initialization success", string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            factory = DDS.DomainParticipantFactory.GetInstance();
            if (factory == null)
            {
                result.Result = "DomainParticipantFactory could not be initialized.";
                return result;
            }

            if (factory.GetDefaultParticipantQos(out pqosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default DomainParticipantQos could not be resolved.";
                return result;
            }
            participant = factory.CreateParticipant(string.Empty, ref pqosHolder, null, 0);
            if (participant == null)
            {
                result.Result = "Creation of DomainParticipant failed.";
                return result;
            }

            if (participant.GetDefaultSubscriberQos(out subQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default SubscriberQos could not be resolved.";
                return result;
            }
            subQosHolder.Partition.Name = new string[1];
            subQosHolder.Partition.Name[0] = "testPartition";
            subscriber = participant.CreateSubscriber(ref subQosHolder);//, null, 0);
            if (subscriber == null)
            {
                result.Result = "Subscriber could not be created.";
                return result;
            }
            typeSupport = new mod.tstTypeSupport();
            rc = typeSupport.RegisterType(participant, "tstType");
            if (rc != DDS.ReturnCode.Ok)
            {
                result.Result = "Typesupport could not be registered.";
                return result;
            }

            if (participant.GetDefaultTopicQos(out tQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default TopicQos could not be resolved.";
                return result;
            }
            tQosHolder.Durability.Kind = DDS.DurabilityQosPolicyKind.TransientDurabilityQos;
            tQosHolder.Reliability.Kind = DDS.ReliabilityQosPolicyKind.ReliableReliabilityQos;
            topic = participant.CreateTopic("tst", "tstType", ref tQosHolder);//, null, 0);
            if (topic == null)
            {
                result.Result = "Topic could not be created.";
                return result;
            }

            if (participant.GetDefaultPublisherQos(out pubQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default PublisherQos could not be resolved.";
                return result;
            }
            pubQosHolder.Partition.Name = new string[1];
            pubQosHolder.Partition.Name[0] = "testPartition";
            publisher = participant.CreatePublisher(ref pubQosHolder);//, null, 0);
            if (publisher == null)
            {
                result.Result = "Publisher could not be created.";
                return result;
            }

            if (publisher.CopyFromTopicQos(out dwQosHolder, ref tQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default DataWriterQos could not be resolved.";
                return result;
            }
            dwQosHolder.WriterDataLifecycle.AutoDisposeUnregisteredInstances = true;
            datawriter = (mod.tstDataWriter)publisher.CreateDataWriter(topic, ref dwQosHolder);//, null, 0);
            if (datawriter == null)
            {
                result.Result = "DataWriter could not be created.";
                return result;
            }

            if (subscriber.CopyFromTopicQos(out drQosHolder, ref tQosHolder) != DDS.ReturnCode.Ok)
            {
                result.Result = "Default DataReaderQos could not be resolved.";
                return result;
            }
            drQosHolder.Durability.Kind = DDS.DurabilityQosPolicyKind.VolatileDurabilityQos;
            testCase.RegisterObject("factory", factory);
            testCase.RegisterObject("participantQos", pqosHolder);
            testCase.RegisterObject("participant", participant);
            testCase.RegisterObject("topic", topic);
            testCase.RegisterObject("topicQos", tQosHolder);
            testCase.RegisterObject("subscriber", subscriber);
            testCase.RegisterObject("subscriberQos", subQosHolder);
            testCase.RegisterObject("datareaderQos", drQosHolder);
            testCase.RegisterObject("publisher", publisher);
            testCase.RegisterObject("publisherQos", pubQosHolder);
            testCase.RegisterObject("datawriter", datawriter);
            testCase.RegisterObject("datawriterQos", dwQosHolder);
            result.Result = "Initialization success.";
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
    }
}